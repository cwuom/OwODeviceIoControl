/**
 * 颜色映射辅助函数
 */
function getColorByIndex(index) {
  if (index == -1) { return '#008c8c'; }
  const teamColors = [
    '#FF0000', '#00FF00', '#0000FF', '#FFFF00', '#FF00FF', '#00FFFF',
    '#FF8000', '#8000FF', '#008080', '#808000', '#FF8080', '#80FF80',
    '#8080FF', '#FFFF80', '#FF80FF', '#80FFFF'
  ];
  const colorIndex = parseInt(index, 10) % 16;
  if (colorIndex < 0 || colorIndex >= teamColors.length) {
    return '#008c8c';
  }
  return teamColors[colorIndex];
}

/**
 * 护甲品质颜色辅助函数
 */
function getQualityColor(level) {
  const colors = [
    '#9d9d9d', // 0: 灰色
    '#ffffff', // 1: 白色
    '#1eff00', // 2: 绿色
    '#0070ff', // 3: 蓝色
    '#a335ee', // 4: 紫色
    '#ff8000', // 5: 橙色
    '#e62020'  // 6: 红色
  ];
  const lvl = parseInt(level, 10);
  if (lvl >= 0 && lvl < colors.length) {
    return colors[lvl];
  }
  return colors[0]; // 默认为灰色
}

// 人员数组
var personMap = new Map();

/**
 * 创建玩家图标 + 信息框的初始HTML
 * @param {string} name - 玩家名称
 * @param {Array} data - [tid, x, y, direciion, operatorName, health, armorHp, armorMaxHp, armorLvl, weapon, helmetHp, helmetMaxHp, helmetLvl]
 */
function createPlayerIconHtml(name, data) {
  //  完整地解构所有数据
  const [
    tid, x, y, direciion, operatorName, health,
    armorHp, armorMaxHp, armorLvl, weapon,
    helmetHp, helmetMaxHp, helmetLvl
  ] = data;

  const color = getColorByIndex(tid);
  const rotation = direciion - dee;

  // 旋转体
  const operatorImg = operatorName && operatorName !== "未知" && operatorName !== "获取失败"
    ? `<img src="/actors/${operatorName}.png" class="actor-image" alt="${operatorName}">`
    : '';

  const markerHtml = `
    <div class="person-marker" style="transform: rotate(${rotation}deg);">
        <div class="arrow" style="background-color: ${color};"></div>
        ${operatorImg}
    </div>
  `;

  // 队伍ID
  const tidHtml = `
    <div class="text" style="color: ${color};">${parseInt(tid) + 1}</div>
  `;

  // 常驻信息框
  const healthPerc = Math.min(100, Math.max(0, parseFloat(health))).toFixed(0);

  const helmetLvlNum = parseInt(helmetLvl, 10) || 0;
  const helmetMaxHpNum = parseFloat(helmetMaxHp) || 0;
  const helmetHpNum = parseFloat(helmetHp) || 0;
  const helmetPerc = (helmetMaxHpNum > 0) ? (Math.min(100, Math.max(0, (helmetHpNum / helmetMaxHpNum) * 100))).toFixed(0) : 0;
  const helmetColorClass = `quality-${helmetLvlNum}`;
  const helmetTextClass = helmetLvlNum === 1 ? 'quality-text-1' : '';

  const armorLvlNum = parseInt(armorLvl, 10) || 0;
  const armorMaxHpNum = parseFloat(armorMaxHp) || 0;
  const armorHpNum = parseFloat(armorHp) || 0;
  const armorPerc = (armorMaxHpNum > 0) ? (Math.min(100, Math.max(0, (armorHpNum / armorMaxHpNum) * 100))).toFixed(0) : 0;
  const armorColorClass = `quality-${armorLvlNum}`;
  const armorTextClass = armorLvlNum === 1 ? 'quality-text-1' : '';
  
  const infoBoxHtml = `
    <div class="info-box">
        <div class="player-name">${name}</div>
        <div class="operator-name">${operatorName}</div>
        
        <div class="bar-container health-bar">
            <div class="bar-fill" style="width: ${healthPerc}%; background-color: #d9534f;"></div>
            <div class="bar-text">${healthPerc} HP</div>
        </div>

        <div class="bar-container helmet-bar">
            <div class="bar-fill ${helmetColorClass}" style="width: ${helmetPerc}%;"></div>
            <div class="bar-text ${helmetTextClass}">L${helmetLvlNum} (${helmetHpNum.toFixed(0)}/${helmetMaxHpNum.toFixed(0)})</div>
        </div>
        
        <div class="bar-container armor-bar">
            <div class="bar-fill ${armorColorClass}" style="width: ${armorPerc}%;"></div>
            <div class="bar-text ${armorTextClass}">L${armorLvlNum} (${armorHpNum.toFixed(0)}/${armorMaxHpNum.toFixed(0)})</div>
        </div>
        
        <div class="weapon-name">${weapon || "近战"}</div>
    </div>
  `;
  
  // 组合并返回
  return markerHtml + tidHtml + infoBoxHtml;
}

/**
 * 创建玩家信息弹窗的 HTML (添加头盔)
 */
function createPlayerPopupHtml(name, operatorName, health, armorHp, armorMaxHp, armorLvl, weapon, helmetHp, helmetMaxHp, helmetLvl) {
  const healthPerc = Math.min(100, Math.max(0, parseFloat(health))).toFixed(0);
  
  const armorMaxHpNum = parseFloat(armorMaxHp) || 0;
  const armorHpNum = parseFloat(armorHp) || 0;
  const armorLvlNum = parseInt(armorLvl, 10) || 0;
  const armorPerc = (armorMaxHpNum > 0) ? (Math.min(100, Math.max(0, (armorHpNum / armorMaxHpNum) * 100))).toFixed(0) : 0;

  const helmetMaxHpNum = parseFloat(helmetMaxHp) || 0;
  const helmetHpNum = parseFloat(helmetHp) || 0;
  const helmetLvlNum = parseInt(helmetLvl, 10) || 0;
  const helmetPerc = (helmetMaxHpNum > 0) ? (Math.min(100, Math.max(0, (helmetHpNum / helmetMaxHpNum) * 100))).toFixed(0) : 0;


  return `
    <div class="player-popup">
        <div class="popup-name">${name}</div>
        <div class="popup-operator">${operatorName}</div>
        <div class="popup-weapon">${weapon || "近战"}</div>
        
        <div class="popup-bar-container">
            <div class="popup-label">HP:</div>
            <div class="popup-bar health-bar">
                <div class="bar-fill" style="width: ${healthPerc}%;"></div>
                <div class="bar-text">${healthPerc} HP</div>
            </div>
        </div>

        <div class="popup-bar-container">
            <div class="popup-label">L${helmetLvlNum}</div>
            <div class="popup-bar helmet-bar">
                <div class="bar-fill" style="width: ${helmetPerc}%; background-color: ${getQualityColor(helmetLvlNum)};"></div>
                <div class="bar-text">${helmetPerc}% (${helmetHpNum.toFixed(0)}/${helmetMaxHpNum.toFixed(0)})</div>
            </div>
        </div>

        <div class="popup-bar-container">
            <div class="popup-label">L${armorLvlNum}</div>
            <div class="popup-bar armor-bar">
                <div class="bar-fill" style="width: ${armorPerc}%; background-color: ${getQualityColor(armorLvlNum)};"></div>
                <div class="bar-text">${armorPerc}% (${armorHpNum.toFixed(0)}/${armorMaxHpNum.toFixed(0)})</div>
            </div>
        </div>
    </div>
  `;
}


/**
 * 添加人员
 */
function addPerson(id, data, colorParams, type) {
  const curTime = Date.now();
  const [tid, x, y] = data;

  const pos = getMapPos(x + xW, y + yW);
  const latLng = [pos.y, pos.x];

  let markerIcon;

  if (type == 1) {
    // 是人
    const iconHtml = createPlayerIconHtml(id, data); //  现在这个 HTML 包含正确的数据

    markerIcon = L.divIcon({
      className: 'person-icon-container',
      html: iconHtml,
      //  调整图标总高度
      iconSize: [80, 125], 
      // 锚点水平居中(40)，垂直保持在玩家脚下(32)
      iconAnchor: [40, 32] 
    });

  } else {
    // 是物质
    const color = tid == 5 ? '#FFCC00' : '#FF0000';
    const iconHtml = `
        <div class="material-marker" style="background-color: ${color};">
            ${id}
        </div>
    `;
    markerIcon = L.divIcon({
      className: 'material-icon-container',
      html: iconHtml,
      iconSize: [20, 20],
      iconAnchor: [10, 10]
    });
  }

  const marker = L.marker(latLng, { 
      icon: markerIcon,
  });

  if (type == 1) {
    const popupHtml = createPlayerPopupHtml(id, ...data.slice(4)); // 传递所需参数
    marker.bindPopup(popupHtml, {
      className: 'player-leaflet-popup',
      offset: [0, -35] // 弹窗偏移
    });
  }

  marker.addTo(map);

  personMap.set(id, {
    marker,
    curTime,
    type,
    currentTid: tid,
    data: data // 存储所有数据
  });
}

let gTid = ''; // 本人的队伍

const userInput = prompt("请输入房间号：", "");
if (userInput && userInput.trim()) {
  const eventSource = new EventSource('http://192.168.0.232:9115/join?roomId=' + userInput.trim());

  eventSource.onmessage = function (event) {
    // 解析16个字段
    const data = event.data.split(",");

    // 核心防闪烁检查
    if (data.length < 16 || data[15] === undefined || data[15] === "") {
      // console.warn("Received malformed or incomplete packet, skipping update:", event.data);
      return; // 丢弃这个坏包，防止闪烁
    }

    const [
        roomId, tid, name, x, y, direciion, type,
        operatorName, health, armorHp, armorMaxHp, armorLvl, weapon,
        helmetHp, helmetMaxHp, helmetLvl
    ] = data;

    const targetName = document.querySelector('.select-iput1').value;
    if (tid == 0) return;

    const per = personMap.get(name);
    
    // 聚合所有需要传递的数据 (索引 0-12)
    const playerData = [
      tid, +x, +y, direciion, operatorName, health, 
      armorHp, armorMaxHp, armorLvl, weapon, // 6, 7, 8
      helmetHp, helmetMaxHp, helmetLvl     // 10, 11, 12
    ];

    // 玩家添加逻辑
    if (targetName && name === targetName && !per) {
      addPerson(name, playerData, '#00FF00', type);
      gTid = tid;
      return;
    }
    if (targetName && name === targetName && tid !== gTid) {
      gTid = tid;
      per && map.removeLayer(per.marker);
      per && personMap.delete(name);
      addPerson(name, playerData, '#00FF00', type);
      return;
    }
    if (targetName && tid === gTid && name !== targetName) {
      // 队友
      per && map.removeLayer(per.marker);
      per && personMap.delete(name);
      return;
    }
    if (!per) {
      addPerson(name, playerData, undefined, type);
      return;
    }


    const curTime = Date.now();
    per.curTime = curTime;
    const pos = getMapPos(+x + xW, +y + yW);
    const latLng = [pos.y, pos.x];

    // 更新位置
    per.marker.setLatLng(latLng);

    if (per.type == 1) {
      
      // 先获取旧数据
      const oldData = per.data || []; 

      const newHealth = parseFloat(health) || 0;
      const newWeapon = weapon || "近战";
      const newOperator = operatorName;

      const newHelmetLvlNum = parseInt(helmetLvl, 10) || 0;
      const newHelmetHp = parseFloat(helmetHp) || 0;
      const newHelmetMaxHp = parseFloat(helmetMaxHp) || 0;

      const newArmorLvlNum = parseInt(armorLvl, 10) || 0;
      const newArmorHp = parseFloat(armorHp) || 0;
      const newArmorMaxHp = parseFloat(armorMaxHp) || 0;

      const oldHelmetLvlNum = parseInt(oldData[12], 10) || 0;
      const oldHelmetMaxHp = parseFloat(oldData[11]) || 0;
      const oldArmorLvlNum = parseInt(oldData[8], 10) || 0;
      const oldArmorMaxHp = parseFloat(oldData[7]) || 0;

      let finalHelmetMaxHp;
      if (newHelmetMaxHp > 0) {
        finalHelmetMaxHp = newHelmetMaxHp;
      } else if (newHelmetLvlNum > 0 && newHelmetLvlNum === oldHelmetLvlNum) {
        finalHelmetMaxHp = oldHelmetMaxHp; // 沿用旧值
      } else {
        finalHelmetMaxHp = newHelmetMaxHp; // 接受新值(0)
      }

      let finalArmorMaxHp;
      if (newArmorMaxHp > 0) {
        finalArmorMaxHp = newArmorMaxHp;
      } else if (newArmorLvlNum > 0 && newArmorLvlNum === oldArmorLvlNum) {
        finalArmorMaxHp = oldArmorMaxHp; // 沿用旧值
      } else {
        finalArmorMaxHp = newArmorMaxHp; // 接受新值(0)
      }

      per.data = [
        tid, +x, +y, direciion, newOperator, newHealth, 
        newArmorHp, finalArmorMaxHp, newArmorLvlNum, newWeapon, // 6, 7, 8
        newHelmetHp, finalHelmetMaxHp, newHelmetLvlNum      // 10, 11, 12
      ];
      per.currentTid = tid;

      const markerElement = per.marker.getElement();
      if (!markerElement) return; 

      const rotation = direciion - dee;
      const rotator = markerElement.querySelector('.person-marker');
      if (rotator) {
        rotator.style.transform = `rotate(${rotation}deg)`;
      }

      // 头盔
      const helmetPerc = (finalHelmetMaxHp > 0) ? (Math.min(100, Math.max(0, (newHelmetHp / finalHelmetMaxHp) * 100))).toFixed(0) : 0;
      const helmetBarFill = markerElement.querySelector('.helmet-bar .bar-fill');
      const helmetBarText = markerElement.querySelector('.helmet-bar .bar-text');
      if (helmetBarFill) {
        helmetBarFill.style.width = `${helmetPerc}%`;
        helmetBarFill.className = `bar-fill quality-${newHelmetLvlNum}`; 
      }
      if (helmetBarText) {
        helmetBarText.innerText = `L${newHelmetLvlNum} (${newHelmetHp.toFixed(0)}/${finalHelmetMaxHp.toFixed(0)})`;
        helmetBarText.className = `bar-text ${newHelmetLvlNum === 1 ? 'quality-text-1' : ''}`;
      }
      
      // 护甲
      const armorPerc = (finalArmorMaxHp > 0) ? (Math.min(100, Math.max(0, (newArmorHp / finalArmorMaxHp) * 100))).toFixed(0) : 0;
      const armorBarFill = markerElement.querySelector('.armor-bar .bar-fill');
      const armorBarText = markerElement.querySelector('.armor-bar .bar-text');
      if (armorBarFill) {
        armorBarFill.style.width = `${armorPerc}%`;
        armorBarFill.className = `bar-fill quality-${newArmorLvlNum}`;
      }
      if (armorBarText) {
        armorBarText.innerText = `L${newArmorLvlNum} (${newArmorHp.toFixed(0)}/${finalArmorMaxHp.toFixed(0)})`;
        armorBarText.className = `bar-text ${newArmorLvlNum === 1 ? 'quality-text-1' : ''}`;
      }

      // 血量
      const healthPerc = Math.min(100, Math.max(0, newHealth)).toFixed(0);
      const healthBarFill = markerElement.querySelector('.health-bar .bar-fill');
      const healthBarText = markerElement.querySelector('.health-bar .bar-text');
      if (healthBarFill) {
        healthBarFill.style.width = `${healthPerc}%`;
      }
      if (healthBarText) {
        healthBarText.innerText = `${healthPerc} HP`;
      }
      
      // 姓名和角色
      const playerNameEl = markerElement.querySelector('.player-name');
      if (playerNameEl) playerNameEl.innerText = name;
      const operatorNameEl = markerElement.querySelector('.operator-name');
      if (operatorNameEl) operatorNameEl.innerText = newOperator;

      // 武器
      const weaponNameEl = markerElement.querySelector('.weapon-name');
      if (weaponNameEl) weaponNameEl.innerText = newWeapon;


      const newPopupHtml = createPlayerPopupHtml(
        name, newOperator, newHealth, 
        newArmorHp, finalArmorMaxHp, newArmorLvlNum, // 护甲使用 finalMax
        newWeapon, 
        newHelmetHp, finalHelmetMaxHp, newHelmetLvlNum // 头盔使用 finalMax
      );
      per.marker.setPopupContent(newPopupHtml);
    } // 结束 if (per.type == 1)

    if (name == (targetName || "").trim()) {
      map.setView(latLng, map.getZoom());
    }
  }; // 结束 eventSource.onmessage

} else {
  console.log("用户取消了输入");
}


setInterval(() => {
  const curTime = Date.now();
  const ior = personMap.keys();
  const arrIds = [];
  for (const name of ior) {
    const oldTime = personMap.get(name).curTime;
    if (curTime - oldTime > (1000 * 5)) {
      arrIds.push(name);
    }
  }
  arrIds.forEach(name => {
    const per = personMap.get(name);
    per && map.removeLayer(per.marker);
    per && personMap.delete(name);
  });
}, 1000);