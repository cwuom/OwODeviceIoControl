/**
 * 颜色映射辅助函数 - 返回50种预定义颜色中的一种
 * @param {number} index - 颜色索引 (0-49)，-1表示特殊颜色
 * @returns {string} - 对应的颜色值 (CSS格式)
 */
function getColorByIndex(index) {
  // 特殊索引处理
  if (index === -1) {
    return '#008c8c'; // 特殊颜色：半透明灰色
  }

  // 确保索引在有效范围内
  index = Math.max(0, Math.min(index, 49));

  // 预定义50种颜色 - 基于彩虹色轮均匀分布
  const colors = [
      '#00FFFF',
      '#FFFF00',
      '#8B3A3A',
      '#FF7F00',
      '#9F79EE',
      '#008B8B',
      '#27f302',
      '#0602f3',
      '#90EE90',
      '#fb07bb',
      '#f70303',
      '#FAFAD2',
  ];

  return colors[index];
}

// 人员数组
var personMap = new Map()

// 添加人员
function addPerson(id, [tid, x, y, direciion], colorParams, type) {
  const curTime = Date.now()
  const pos = getMapPos(x + xW, y + yW)
  
  const svgElement = document.createElementNS("http://www.w3.org/2000/svg", "svg");
  svgElement.setAttribute('xmlns', "http://www.w3.org/2000/svg");
  svgElement.setAttribute('viewBox', "0 0 10000 10000");
  svgElement.style.overflow = 'visible'

  if( type == 1) {
    // 是人
    const color = colorParams || getColorByIndex(tid)
    // [!code focus:5]
    svgElement.innerHTML = `
    <line x1="5000" y1="0" x2="5000" y2="10000" 
            stroke="${color}" stroke-width="1000" />
    <circle cx="5000" cy="7000" r="3000" fill="${color}" stroke="black" stroke-width="0" />
    <text x="5000" y="9000" font-family="Arial" font-size="5000" font-weight="bold"
          text-anchor="middle" fill="white">${tid}</text>
    `;
  } else {
    // 是物质
    const color = tid == 5 ? '#FFCC00' : '#FF0000'
    // [!code focus:3]
    svgElement.innerHTML = `
    <circle cx="5000" cy="7000" r="3000" fill="${color}" stroke="black" stroke-width="0" />
    <text x="5000" y="9000" font-family="Arial" font-size="4000" font-weight="bold"
          text-anchor="middle" fill="white">${id}</text>
    `;
  }
  

  const svgElementBounds = [[pos.y, pos.x], [pos.y - 2, pos.x + 2]];
  const marker = L.svgOverlay(svgElement, svgElementBounds)
  marker.addTo(map)
  personMap.set(id, {
    marker,
    curTime,
    svgElement
  })

  
  if(type == 1) {
    // const currentTransform = svgElement.style.transform || ''; // <-- 旧代码
    // 追加新的旋转变换 // <-- 旧代码
    // svgElement.style.transform = `${currentTransform} rotate(${direciion - dee}deg)`; // <-- 旧代码
    
    // [!code focus:2]
    // 修正：直接设置旋转，而不是累加
    svgElement.style.transform = `rotate(${direciion - dee}deg)`;
    svgElement.style['transform-origin'] = `50% 80%`; // 这个现在是正确的
  }
}

let gTid = ''  // 本人的队伍

const userInput = prompt("请输入房间号：", "");
if (userInput.trim()) {
    //console.log("用户输入：", userInput.trim());
    // 创建SSE连接
    //const eventSource = new EventSource('http://8.148.74.159:9115/join?roomId=' + userInput.trim());
    const eventSource = new EventSource('http://127.0.0.1:9115/join?roomId=' + userInput.trim());
    // 监听普通消息
    eventSource.onmessage = function (event) {
      const [roomId, tid, name, x, y, direciion, type] = event.data.split(",")
      const targetName = document.querySelector('.select-iput1').value
      if(tid == 0) return;
      console.log(event.data);
      const per = personMap.get(name)
      if(targetName && name === targetName && !per) {
        addPerson(name, [tid, +x, +y, direciion], '#00FF00', type)
        return;
      }
      if(targetName && name === targetName && tid !== gTid) {
        gTid = tid
        per && map.removeControl(per.marker)
        per && personMap.delete(name)
        addPerson(name, [tid, +x, +y, direciion], '#00FF00', type)
        return;
      } 
      if(targetName && tid === gTid && name !== targetName) {
        // 队友
        per && map.removeControl(per.marker)
        per && personMap.delete(name)
        return;
      }
      if (!per) {
        addPerson(name, [tid, +x, +y, direciion], undefined, type)
        return;
      }

      const curTime = Date.now()
      per.curTime = curTime
      const pos = getMapPos(+x + xW, +y + yW)
      per.marker.setBounds([[pos.y, pos.x], [pos.y - 2, pos.x + 2]])

      if(type == 1) {
        const currentTransform = per.svgElement.style.transform || '';
        per.svgElement.style.transform = `${currentTransform} rotate(${direciion - dee}deg)`;
      }
      
      if(name==(targetName || "").trim()){
        map.setView([pos.y, pos.x], map.getZoom())
      }
    };
} else {
    console.log("用户取消了输入");
}


setInterval(() => {
  const curTime = Date.now()
  const ior = personMap.keys()
  const arrIds = []
  for(const name of ior) {
    const oldTime = personMap.get(name).curTime
    if( curTime - oldTime > (1000 * 5) ) {
      arrIds.push(name)
    }
  }
  arrIds.forEach(name => {
    const per = personMap.get(name)
    per && map.removeControl(per.marker)
    per && personMap.delete(name)
  })
}, 1000)


