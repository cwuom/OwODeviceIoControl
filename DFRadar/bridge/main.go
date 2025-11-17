package main

import (
	"fmt"
	"log"
	"net"
	"net/http"
	"strings"
	"sync"
	"time"
)

const (
	HTTP_PORT = "9115" // 网页雷达访问的端口
	UDP_PORT  = "9116" // C++ 程序发送数据的端口
)

// Client代表一个连接的网页客户端
// 使用一个带缓冲的 channel 来避免 UDP 广播时被卡住
type Client chan string

// RoomManager 线程安全地管理所有房间和客户端
type RoomManager struct {
	rooms map[string]map[Client]bool
	mutex sync.RWMutex
}

// NewRoomManager 创建一个新的 RoomManager
func NewRoomManager() *RoomManager {
	return &RoomManager{
		rooms: make(map[string]map[Client]bool),
	}
}

// AddClient 将一个客户端（的 channel）添加到一个房间
func (rm *RoomManager) AddClient(roomId string, client Client) {
	rm.mutex.Lock()
	defer rm.mutex.Unlock()

	if _, ok := rm.rooms[roomId]; !ok {
		rm.rooms[roomId] = make(map[Client]bool)
	}
	rm.rooms[roomId][client] = true
	log.Printf("[HTTP] 客户端加入房间: %s (当前房间人数: %d)\n", roomId, len(rm.rooms[roomId]))
}

// RemoveClient 从一个房间移除客户端
func (rm *RoomManager) RemoveClient(roomId string, client Client) {
	rm.mutex.Lock()
	defer rm.mutex.Unlock()

	if room, ok := rm.rooms[roomId]; ok {
		if _, ok := room[client]; ok {
			delete(room, client)
			close(client) // 关闭 channel
			log.Printf("[HTTP] 客户端离开房间: %s (剩余人数: %d)\n", roomId, len(rm.rooms[roomId]))
		}
		if len(room) == 0 {
			delete(rm.rooms, roomId)
		}
	}
}

// BroadcastMessage 向一个房间的所有客户端广播消息
func (rm *RoomManager) BroadcastMessage(roomId string, message string) {
	rm.mutex.RLock()
	defer rm.mutex.RUnlock()

	if room, ok := rm.rooms[roomId]; ok {
		for client := range room {
			// 使用 select 和 default 来实现非阻塞发送
			// 如果客户端的 channel 满了（处理不过来），就丢弃这条消息
			select {
			case client <- message:
			default:
				log.Printf("[UDP] 客户端 %s 的缓冲区已满, 丢弃消息\n", roomId)
			}
		}
	}
}

// sseHandler 处理 /join 的 SSE (Server-Sent Events) 请求
func (rm *RoomManager) sseHandler(w http.ResponseWriter, r *http.Request) {
	// 从 URL query 获取 roomId
	roomId := r.URL.Query().Get("roomId")
	if roomId == "" {
		http.Error(w, "Missing roomId", http.StatusBadRequest)
		return
	}

	// 设置 SSE 头部
	w.Header().Set("Content-Type", "text/event-stream")
	w.Header().Set("Cache-Control", "no-cache")
	w.Header().Set("Connection", "keep-alive")
	w.Header().Set("Access-Control-Allow-Origin", "*") // 允许跨域

	// 获取 http.Flusher，这是 SSE 的关键
	flusher, ok := w.(http.Flusher)
	if !ok {
		http.Error(w, "Streaming unsupported!", http.StatusInternalServerError)
		return
	}

	// 为这个客户端创建一个消息 channel
	messageChan := make(Client, 2048)

	// 将客户端添加到房间
	rm.AddClient(roomId, messageChan)

	// 当客户端断开连接时，自动移除
	ctx := r.Context()
	defer rm.RemoveClient(roomId, messageChan)

	// 循环监听 channel 和断开连接事件
	for {
		select {
		case <-ctx.Done(): // 客户端断开连接
			return
		case message := <-messageChan: // 从 UDP 收到新消息
			fmt.Fprintf(w, "data: %s\n\n", message)
			flusher.Flush()
		case <-time.After(15 * time.Second): // 定期心跳保持连接
			fmt.Fprintf(w, "event: ping\ndata: {}\n\n")
			flusher.Flush()
		}
	}
}

// startUDPServer 启动 UDP 服务器，在一个单独的 goroutine 中运行
func (rm *RoomManager) startUDPServer(port string) {
	addr, err := net.ResolveUDPAddr("udp4", ":"+port)
	if err != nil {
		log.Fatalf("[UDP] 解析地址失败: %v", err)
	}

	conn, err := net.ListenPacket("udp4", ":"+port)
	if err != nil {
		log.Fatalf("[UDP] 监听 UDP 失败: %v", err)
	}
	defer conn.Close()

	log.Printf("[UDP] 正在监听来自 C++ 的数据: %s\n", addr.String())

	buffer := make([]byte, 2048) // 2KB 缓冲区

	for {
		n, _, err := conn.ReadFrom(buffer)
		if err != nil {
			log.Printf("[UDP] 读取错误: %v", err)
			continue
		}

		message := string(buffer[:n])
		
		parts := strings.Split(message, ",")
		if len(parts) < 7 {
			log.Printf("[UDP] 收到格式错误的数据包: %s\n", message)
			continue
		}

		roomId := parts[0]
		// 广播给房间里的所有人
		rm.BroadcastMessage(roomId, message)
	}
}

func main() {
	roomManager := NewRoomManager()

	go roomManager.startUDPServer(UDP_PORT)

	http.HandleFunc("/join", roomManager.sseHandler)
	
	http.Handle("/", http.FileServer(http.Dir(".")))

	log.Printf("[HTTP] Web 雷达服务器启动于 http://127.0.0.1:%s\n", HTTP_PORT)
	log.Fatal(http.ListenAndServe(":"+HTTP_PORT, nil))
}