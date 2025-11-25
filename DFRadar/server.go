package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
)

func main() {
	port := "5500"

	dir, err := os.Getwd()
	if err != nil {
		log.Fatal(err)
	}

	fileServer := http.FileServer(http.Dir("."))

	http.Handle("/", fileServer)

	fmt.Println("------------------------------------------------")
	fmt.Printf("Web 服务器已启动，根目录: %s\n", dir)
	fmt.Printf("本地访问: http://localhost:%s\n", port)
	fmt.Println("正在监听所有网络接口 (IPv4 & IPv6)...")
	fmt.Println("------------------------------------------------")

	err = http.ListenAndServe(":"+port, nil)
	if err != nil {
		log.Fatal("服务器启动失败: ", err)
	}
}