/* eslint-disable */

var getQuery = function (name) {
    var m = window.location.search.match(new RegExp('(\\?|&)' + name + '=([^&]*)(&|$)'));
    return !m ? '' : decodeURIComponent(m[2]);
};

function debounce(callback, delay) {
    let timerId;
   
    return function(...args) {
      clearTimeout(timerId);
   
      timerId = setTimeout(() => {
        callback.apply(this, args);
      }, delay);
    };
  }

function copyToClipboard (text) {
    if (navigator.clipboard && navigator.clipboard.writeText) {
        navigator.clipboard.writeText(text).then(function() {
            var msg = text.length > 15 ? text.slice(0, 15) + '...' : text;
            // // console.log('已经'+msg+'复制到剪贴板！');
        }).catch(function(err) {
            // // console.error('无法复制到剪贴板', err);
        });
    } else {
        let textArea = document.createElement('textarea');
        textArea.value = text;
        textArea.style.position = 'fixed';
        document.body.appendChild(textArea);
        textArea.focus();
        textArea.select();
        try {
            document.execCommand('copy');
            var msg = text.length > 15 ? text.slice(0, 15) + '...' : text;
            // // console.log('已经'+msg+'复制到剪贴板！');
        } catch (err) {
            // // console.error('无法复制到剪贴板', err);
        }
        document.body.removeChild(textArea);
    }
}

var browser = {
    versions: (function () {
        var u = navigator.userAgent;
        return {
            mobile: !!u.match(/AppleWebKit.*Mobile.*/), // 移动终端
            Tablet: u.indexOf('Tablet') > -1 || u.indexOf('Pad') > -1 || u.indexOf('Nexus 7') > -1, // 平板
            ios: u.indexOf('like Mac OS X') > -1, // ios终端
            android: u.indexOf('Android') > -1 || u.indexOf('Adr') > -1, // android终端
            Safari: u.indexOf('Safari') > -1,
            Chrome: u.indexOf('Chrome') > -1 || u.indexOf('CriOS') > -1,
            IE: u.indexOf('MSIE') > -1 || u.indexOf('Trident') > -1,
            Edge: u.indexOf('Edge') > -1,
            QQBrowser: u.indexOf('QQBrowser') > -1,
            QQ: u.indexOf('QQ/') > -1,
            Wechat: u.indexOf('MicroMessenger') > -1,
            Weibo: u.indexOf('Weibo') > -1,
            360: u.indexOf('QihooBrowser') > -1,
            UC: u.indexOf('UC') > -1 || u.indexOf(' UBrowser') > -1,
            Taobao: u.indexOf('AliApp(TB') > -1,
            Alipay: u.indexOf('AliApp(AP') > -1,
            isMac: /macintosh|mac os x/i.test(navigator.userAgent),
            isSafari: /Safari/.test(u) && !/Chrome/.test(u)
        };
    })(),
    language: (navigator.browserLanguage || navigator.language).toLowerCase()
};



/**变量 */
const navCtn = $('.left-nav-ctn');
let pageSwiper = null;
let part3ListTop = true;
let part3ListBot = false;
let movePart3List = false;
let currScrollIndex = 0;
let footShow = false;

const warLvText = [
    'A','B','C','D','E'
]

var queryMap = {
    'daba': '00',
    'cgxg': '10',
    'htjd': '21',
    'bks': '31',
    'cxjy': '42'
}

// 全面战场新全局变量
window.viewChange = true; // 进攻方视角

window.occupy = false; // 占领模式

window.warLv = 0; // 阶段

window.isLvChange = false;

window.warSwiper = null; // 部署swiper

window.pervInitX = '' // 上一次位移

// 导航相关
var navTypyList = $('.nav-type-list')
var regionList = $('.region-list')
var floorList = $('.floor-list')
var currLeftNav = 0;

// 地图相关
var visibleMarker = {};
var listIsAll = {
    0: false,
    1: false,
    2: false,
    3: false,
    4: false,
    5: false,
    6: false,
    7: false,
    8: false,
}
var visibleMarker2 = {
    0: {
        isAll: false,
        isInit: false,
        markers: {}
    },
    1: {
        isAll: false,
        isInit: false,
        markers: {}
    },
    2: {
        isAll: false,
        isInit: false,
        markers: {}
    },
    3: {
        isAll: false,
        isInit: false,
        markers: {}
    },
    4: {
        isAll: false,
        isInit: false,
        markers: {}
    },
    5: {
        isAll: false,
        isInit: false,
        markers: {}
    },
    6: {
        isAll: false,
        isInit: false,
        markers: {}
    },
    7: {
        isAll: false,
        isInit: false,
        markers: {}
    },
    8: {
        isAll: false,
        isInit: false,
        markers: {}
    }
};
var hoverMarker = {};
var clickMarker = {}
var ciLayer = null;
var typeListInit = false;
var NavCliciIndex = 1;
function Page() {
    var _this = this;
    _this.$page = $('.m-index')

    // _this.$page.css('height', window.innerHeight)
    _this.init = function () {
        // // console.log(11111);
        $('.global-loading').hide();
        $('.region-ctn').show();
        // // console.log('init');
        if (!browser.versions.mobile) {
           
            _this.resizeDom();
            if (window.innerWidth / window.innerHeight > 1920 / 1080) {
                $('.select_map_video').css({ 'width': '100%', 'height': 'auto'})
            } else {
                $('.select_map_video').css({ 'width': 'auto', 'height': '100%'})
            }
        }
       
        _this.isInit = true;
    }
    _this.sizeList = { 'ar' : true, 'en': true, 'zh-tw': true, 'ko': true, 'tr': true}
    var sizeAutoList = $('.sizeAuto');
    var scaleAutoList = $('.scaleAuto')
    _this.resizeDom = () => {
        if (window.innerHeight > window.innerWidth) return;
        var size = window.innerWidth / 1920  > 1 ? 1 : window.innerWidth / 1920;
        if (window.innerWidth / window.innerHeight - 1920 / 1080 > 0) {
            // let scale = window.innerWidth / window.innerHeight - 1920 / 1080
            // scaleAutoList.css('bottom', `${scale * 250}px`)
            // topCtn[0].style.top = `${scale * 250}px`
        } else {
            scaleAutoList.css('bottom', '0px')
        }

        
    };

    window.onresize = function(e) {
        if (!browser.versions.mobile) {
            _this.resizeDom();
        }
    }


}

var bksTop = ['-461305', '-460454', '-459334', '-460257', '-459631', '-459328.9688', '-458885', '-459003', '-458692']
var bksBom = ['-458000', '-457863', '-457854', '-457554', '-457310', '-457776', '-457320', '-457322', '-457830']

var mapScaleInfo = dabaInfo;
// var mapScaleInfo = gcInfo;
function getMapPos (posX, posY) {
    
    if (currLayer.name === 'bks_1f' || currLayer.name === 'map_bks2') {
        // console.log(posX, posY);
        
        if (bksTop.includes(posY)) {
            posX = Number(posX) + 700
            posY = Number(posY) - 200
        }
        if (bksBom.includes(posY)) {
            posX = Number(posX) + 900
            posY = Number(posY) + 200
        }
        if (posY === -459085) {
            posX = Number(posX) + 300
        }

    }
    var x = Number(posX)
    var y = Number(posY)
    var bj = isFloor ? mapScaleInfo.floorInfo.info.bj : 128
    // x轴转换计算公式：世界轴 / 设计稿宽度/2
    // x轴倍率：81086.304688 / 4096 = 19.79646110546875
    // var xB = 81086.304688 / 4096
    // 81086.304688 / 128
    var xB2 = mapScaleInfo.width / bj

    // y轴计算公式：世界轴 / 设计稿宽度/2
    // y轴倍率：80988.500000 / 4096 = 19.7725830078125
    // var yB = 80988.500000 / 4096 / -bj
    var yB2 = mapScaleInfo.height / bj
    
    // 世界中心轴x： 358155.687500； y： 750191.750000
    // return {x: bj - (mapScaleInfo.centerX - x ) / xB2, y: -bj - (mapScaleInfo.centerY + y ) / yB2}
    // currLayer.name === 'map_gc'|| currLayer.name === 'map_pc'

    if (currLayer.name.indexOf('cgxg') !== -1 || currLayer.name === 'map_yc2' || currLayer.name === 'map_yc'|| mapScaleInfo.rotate) {

        return {x: bj - (mapScaleInfo.centerY + y ) / yB2, y: -bj + (mapScaleInfo.centerX - x ) / xB2}
    } else{
        return {x: bj - (mapScaleInfo.centerX - x ) / xB2, y: -bj - (mapScaleInfo.centerY + y ) / yB2}
    }
    // return {x: 127, y: -68}
}

new Page().init();

        // 项目初始化的一些函数
        var initProject = function () {
            // 阻止微信下拉；原生js绑定覆盖zepto的默认绑定
            // document.body.addEventListener('touchmove', function (e) {
            //     e.preventDefault();
            // }, { passive: false });

            /** 解决ios12微信input软键盘收回时页面不回弹，兼容动态添加dom(腾讯登录组件)的情况 */
            var resetScroll = (function () {
                var timeWindow = 500;
                var timeout; // time in ms
                var functionName = function (args) {
                    let inputEl = $('input, select, textarea');
                    // TODO: 连续添加元素时，可能存在重复绑定事件的情况
                    inputEl && inputEl.on('blur', () => {
                        var scrollHeight = document.documentElement.scrollTop || document.body.scrollTop || 0;
                        window.scrollTo(0, Math.max(scrollHeight, 0));
                    });
                };
        
                return function () {
                    clearTimeout(timeout);
                    timeout = setTimeout(function () {
                        functionName.apply();
                    }, timeWindow);
                };
            }());
        };
    initProject();

    var getImgName = function (str, search) {
        let lastIndex = str.lastIndexOf(search);
        let newStr = str.slice(lastIndex + 1)
        return newStr.split('.')[0];
    }

// 当前地图信息
// 全部icon
var allNavList = navList;
// 单个大类
var navTypeList = navListInfo;
// icon地图映射
var mapIcons = mapArticle;

var poiInfo = selectRegion;

// var allNavList = navList_cgxg;
// // 单个大类
// var navTypeList = navListInfo_cgxg;
// // icon地图映射
// var mapIcons = mapArticle_cgxg;

var isRemove = false;

var poiList = []

// 定义名称与类名的映射，实现可扩展性
const nameClassMap = {
    '保险柜': 'red',
    '小保险箱': 'red',
    '服务器': 'orange',
    '电脑': 'orange'
};

// 随机事件
var randomEvent = {
    // '0': {
    //     name: '炸毁大坝',
    //     lv: '0'
    // },
    '1': {
        name: '坠机事件',
        lv: '0'
    },
    '2': {
        name: '断桥事件',
        lv: '0'
    }
}


// 当前地图
var currLayer;

// 切换地图
var dom_changeMapBtn = $('.btn-change-map-ctn')
var dom_mapList = $('.map-list-ctn')
var isMoveMapList = false;
var currMap = '0';
var currLv = '0';
var currWarMap = 'pc';
var currWarType = 'pc';
var isZj = false;
var currRegion = ''; // 当前切换的区域
var saveMarker = {}; // 切换楼层前保存的点位
var currNavIcon = ''; // 当前选择的icon
var chooseItemLvName = ''; // 选择的难度名称

// 全面战场模式
var isWar = false;
// 攻守方
var isAttack = true;

// 楼层模式
var isFloor = false;
var outFloor = true;
var currFloorIndex = -1;
var currMapFloor = dabaFloor;
var isMoveFloor = false;

// 全选
var isAll = false;

// 标点
var map
var mapFolder = '0_4/'
var cacheMarker = [];
var removeCacheMarker = [];
var markerList = [];
var currClickMarker;
var warMark = [];
var borderList = [];

// 检测楼层点位

// 定义多个兴趣点
// const pointsOfInterest = [
//     { x: 364598.812500, y: -787795.000000, name: "行政辖区", threshold: 10 },
//     // { x: 1500, y: 2500, name: "位置B", threshold: 150 },
//     // { x: 2000, y: 1800, name: "位置C", threshold: 80 }
// ];
var pointsOfInterest = {
    '0': { x: 364598.812500, y: -787795.000000, name: "行政辖区", threshold: 10 },
    '1': { x: 360745.000000, y: -630639.000000, name: "钻石皇后酒店", threshold: 10 },
    '3': { x: 378916.156250, y: -459727.375000, name: "皇家博物馆", threshold: 10 },
    '4': [{ x: 50681.226562, y: -46910.406250, name: "行政区", threshold: 10 },
        { x: 39098.882812, y: -46719.191406, name: "卸货区", threshold: 10 },
        { x: 54663.746094, y: -53846.558594, name: "东侧上层入口", threshold: 10 },
        { x: 45505.500000, y: -53846.558594, name: "西侧上层入口", threshold: 10 },
        { x: 50694.976562, y: -54296.472656, name: "电梯井", threshold: 10 },
        { x: 56148.140625, y: -46268.132812, name: "医疗实验区", threshold: 10 },
        { x: 61580.792969, y: -49258.835938, name: "禁闭区", threshold: 10 },
        { x: 59362.125000, y: -57157.156250, name: "牢房", threshold: 10 },
        { x: 62977.304688, y: -63904.835938, name: "施工区", threshold: 10 },
        { x: 51696.847656, y: -65976.921875, name: "潮汐控制室", threshold: 10 },
        { x: 75549.390625, y: -56491.523438, name: "东侧小岛", threshold: 10 },
        { x: 65006.808594, y: -36544.617188, name: "东瞭望台区", threshold: 10 },
        { x: 36673.597656, y: -39682.421875, name: "西瞭望台区", threshold: 10 },
        { x: 36673.597656, y: -39682.421875, name: "西瞭望台区", threshold: 10 },
        { x: 54200.113281, y: -33264.484375, name: "蓄水区", threshold: 10 },
        { x: 42722.664062, y: -60414.343750, name: "囚犯活动区", threshold: 10 },
        { x: 39293.273438, y: -57157.156250, name: "水动力渠", threshold: 10 },
    ],
}
    


    function refreshMarker2(from, arr) {
        $.each(cacheMarker, function () {
            if (this.options.icon?.polyline) {
                this.options.icon?.polyline.remove();
                this.options.icon?.polyline2.remove();
            }
            this.remove();
        
        });
        // $.each(borderList, function () {
        //     this.remove();
        
        // });
        
        
        isRemove = true;
        cacheMarker = [];
        markerList = []

        
        $.each(arr, function (index, item) {
            var visible = false;
            var that = this;
            if (from === "filter" && visibleMarker[item.name]) visible = true;
            if (from === "filter" && visibleMarker['出生点'] && item.type === 'revive') {
                visible = true;
            }
            if (from === "filter" && visibleMarker['首领'] && item.type === 'Boss') {
                visible = true;
            }
            if (from === "filter" && (visibleMarker['行动接取站']) && item.type === 'move') {
                visible = true;
            }
            if (from === "filter" && (visibleMarker['高价值']) && item.type === 'move') {
                visible = true;
            }
            // // console.log(this);
            
            if (visible) {
                if (item['随机']) {
                    var popupHtml = mapScaleInfo?.floor ? `
                    <div class="name ${(this.name === '[地狱黑鲨]雷斯—雷达站摧毁者' || this.name === '[地狱黑鲨]雷斯—酒店守卫者') && 'max'}">${this.name}<span> ( ${item['随机']} )</span></div>
                    <div class="btn-floor" data-nav=nav-list-nav_${this.icon} data-name=${this.name} data-floor=${mapScaleInfo?.floor[this.floor]?.floor_f} data-index=${this.floor}></div>
                    <div class="address">地点：<span>${item['自定义区域']}</span></div>
                    <div class="open-text ${(!item['拾取条件'] || item['拾取条件'] === '') ? 'hide': ''}">开启条件：<span>${item['拾取条件']}</span></div>
                    <div class="open-text ${(!item['撤离条件'] || item['撤离条件'] === '') ? 'hide': ''}">撤离条件<span>${item['撤离条件']}</span></div>` :
                    ` <div class="name ${(this.name === '[地狱黑鲨]雷斯—雷达站摧毁者' || this.name === '[地狱黑鲨]雷斯—酒店守卫者') && 'max'}">${this.name}<span> ( ${item['随机']} )</span></div>
                    <div class="address">地点：<span>${item['自定义区域']}</span></div>
                    <div class="open-text ${(!item['拾取条件'] || item['拾取条件'] === '') ? 'hide': ''}">开启条件：<span>${item['拾取条件']}</span></div>
                    <div class="open-text ${(!item['撤离条件'] || item['撤离条件'] === '') ? 'hide': ''}">撤离条件：<span>${item['撤离条件']}</span></div>`;
                } else if (item['自定义区域']) {
                    var popupHtml = mapScaleInfo?.floor ? `
                    <div class="name ${(this.name === '[地狱黑鲨]雷斯—雷达站摧毁者' || this.name === '[地狱黑鲨]雷斯—酒店守卫者') && 'max'}">${this?.sub_name ? this.sub_name : this.name}</div>
                    <div class="btn-floor" data-nav=nav-list-nav_${this.icon} data-name=${this.name} data-floor=${mapScaleInfo?.floor[this.floor]?.floor_f} data-index=${this.floor}></div>
                    <div class="address">地点：<span>${item['自定义区域']}</span></div>
                    <div class="open-text ${(!item['拾取条件'] || item['拾取条件'] === '') ? 'hide': ''}">开启条件：<span>${item['拾取条件']}</span></div>
                    <div class="open-text ${(!item['撤离条件'] || item['撤离条件'] === '') ? 'hide': ''}">撤离条件：<span>${item['撤离条件']}</span></div>` : 
                    ` <div class="name ${(this.name === '[地狱黑鲨]雷斯—雷达站摧毁者' || this.name === '[地狱黑鲨]雷斯—酒店守卫者') && 'max'}">${this?.sub_name ? this.sub_name : this.name}</div>
                    <div class="address">地点：<span>${item['自定义区域']}</span></div>
                    <div class="open-text ${(!item['拾取条件'] || item['拾取条件'] === '') ? 'hide': ''}">开启条件：<span>${item['拾取条件']}</span></div>
                    <div class="open-text ${(!item['撤离条件'] || item['撤离条件'] === '') ? 'hide': ''}">撤离条件：<span>${item['撤离条件']}</span></div>`;
                } else if (item['激活条件']) {
                    var popupHtml =`
                            <div class="name">${this.name}</div>
                         <div class="open-text war ${(!item['激活条件'] || item['激活条件'] === '' || item['激活条件'] === '-') ? 'hide': ''}">激活条件：<span>${item['激活条件']}</span></div>
                         `
                   
                } else {
                    var popupHtml = `
                    <div class="name">${this.name}</div>
                `;
                }
              


                popupHtml += '</div>';

                var className = ''
                if (item?.type) {
                    className =  item.type
                } else {
                    className =  'article'
                }
                var pos = getMapPos(this.x, this.y)
                // console.log(this.x, this.y, pos);
                
var path = isWar ? '//game.gtimg.cn/images/dfm/cp/a20240729directory/img/dzc_i/' : '//game.gtimg.cn/images/dfm/cp/a20240729directory/img/lv3/'
                var iconName;
                if (that.name === "进攻方基地" ) {
                    iconName = window.viewChange ? 'g_jdbsd_g': 'g_jdbsd_r'
                } else if (that.name === "防守方基地") {
                    iconName = window.viewChange ? 'f_jdbsd_r': 'f_jdbsd_g'
                } else {
                    iconName = that.icon
                }
                
                if (this.icon) {
                    // style=`transform: translate3d(-50%, -50%, 0) rotate(${element.rotate}deg)`
                    let rotate = currWarMap === 'qhz' ? 90 : 180
                    // console.log('rotate', currWarMap, NavCliciIndex);
                    var myIcon =  L.divIcon({
                        className: ` ${isWar ? 'map-war-icon' : 'map-icon'} ${nameClassMap[that.name] || ''}`,
                        html: `<div class="map-icon-bg" style="${that?.rotate ? `transform: translate3d(-50%, -50%, 0) rotate(${Number(that?.rotate) + rotate}deg) ` : ''}"><img src="${path + iconName}.png"/></div>`,
                        iconSize: [30, 30],			//设置图标大小
                        iconAnchor: [15, 15],		//设置图标偏移
                    })
                    // console.log(that);
                    
                    if (that.name === '电梯撤离点' && that.point1) {
                        var pos1 = getMapPos(that.point1.x, that.point1.y)
                        var pos2 = getMapPos(that.point2.x, that.point2.y)
                        var latlngs1 = [
                            [pos1.y, pos1.x],
                            [pos.y, pos.x]  // 添加终点坐标
                        ];
                        var latlngs2 = [
                            [pos2.y, pos2.x],
                            [pos.y, pos.x]  // 添加终点坐标
                        ];
                        
                        myIcon.polyline = L.polyline(latlngs1, {
                            color: '#EAEBEB',
                            dashArray: '10, 10',  // 虚线样式：10px线段，10px间隔
                            weight: 2
                        }).addTo(map);
                        myIcon.polyline2 = L.polyline(latlngs2, {
                            color: '#EAEBEB',
                            dashArray: '10, 10',  // 虚线样式：10px线段，10px间隔
                            weight: 2
                        }).addTo(map);
                    }
                    myIcon.name = that.name;
                    // console.log('myIcon', that.name);
                    
                    var marker = L.marker([pos.y, pos.x], {icon:  myIcon, zIndexOffset: that.name === currNavIcon ? NavCliciIndex + 1 : NavCliciIndex}).bindPopup(popupHtml).addTo(map).on({
                        click: function () {
                            document.getElementById('MapContainer').classList.remove('zooming');
                            currClickMarker?.setIcon(currClickMarker?.myIcon)
                            this.isClick = true;
                            console.log(that['z坐标'], that.name, that.y);
                            let rotate = currWarMap === 'qhz' ? 90 : 180
                            this.myIcon = myIcon;
                            if (that?.floor || that?.floor === 0) {
                                $('.leaflet-popup').addClass('floor')
                            } else {
                                $('.leaflet-popup').removeClass('floor')
                            }
                            $('.leaflet-popup-close-button').html('')
                            this.openPopup();
                            this.setIcon( L.divIcon({
                                className: ` ${isWar ? 'map-war-icon' : 'map-icon'} click ${nameClassMap[that.name] || ''}`,
                                html: `<div class="map-icon-bg" ><img src="${path + iconName}.png" style="${that?.rotate ? `transform:  rotate(${Number(that?.rotate) + rotate}deg)` : ''}"/></div>`,
                                iconSize: [30, 30],			//设置图标大小
                                iconAnchor: [15, 15],		//设置图标偏移
                            }));
                            currClickMarker = this;
                            $(this.getElement()).addClass('click')
                            console.log(this.myIcon, item);
                            if (this.myIcon.name.indexOf('基地') > -1 || (this.myIcon.name.indexOf('据点') > -1 && window.occupy)) {
                                initWarSwiper(this.myIcon.name, that);
                            }


                            $('.btn-floor').on('click', enterFloorMode)
                            // this?.remove()
                        }
                    })
                    // console.log( marker);
                    // marker.setZIndexOffset(NavCliciIndex)
                    // marker.getElement().style.zIndex = NavCliciIndex;
                    cacheMarker.push(marker)
                }
                
            }
          
        });
        isRemove = false;
        // console.log(cacheMarker.length);
    }



    function toggleVisible(type, index) {
        
        switch (type) {
            case "0_all":
            case "1_all":
            case "2_all":
            case "3_all":
            case "4_all":
            case "5_all":
                renderMarker()
                break;
            case "0_none":
            case "1_none":
            case "2_none":
            case "3_none":
            case "4_none":
            case "5_none":
              
                renderMarker()
                isWar ? $('.map-war-icon').remove() : $('.map-icon').remove();
                break;
            case "none":
                isWar ? $('.map-war-icon').remove() : $('.map-icon').remove();
                
                currClickMarker?.remove();
                renderMarker2()
                break;
        
            default:
                visibleMarker[type] = visibleMarker[type] ? false : true;
                break;
        }
        $('.deploy-swiper').removeClass('show')
        // console.log('type', type);
        function renderMarker () {
            if (Number(currLeftNav) === 0) {
                // console.log(111, navTypeList, visibleMarker);
                
                for (var o in visibleMarker) {
                    if (visibleMarker.hasOwnProperty(o)) {
                        visibleMarker[o] = (type.indexOf('all') > 0 ? true : false);
                    }
                }
            } else {
                for (let index = 0; index < navTypeList[currLeftNav].typeList.length; index++) {
                    const element = navTypeList[currLeftNav].typeList[index];
                    // // console.log(element);
                    // if (element.num === 0) return;
                    
                    visibleMarker[element.name] = (type.indexOf('all') > 0 ? true : false);
                }
            }
            saveMarker = Object.assign({}, visibleMarker);
        }

        function renderMarker2 () {
            for (var o in visibleMarker) {
                if (visibleMarker.hasOwnProperty(o)) {
                    visibleMarker[o] = (type.indexOf('all') > 0 ? true : false);
                }
            }
            saveMarker = Object.assign({}, visibleMarker);
        }

        refreshMarker2("filter", mapIcons);
    }




var init = function () {
    // var mapWidth = -250;  
    // var mapHeight = 250;  

    var mapWidth = isFloor ? mapScaleInfo.floorInfo.info.boundsW : mapScaleInfo.boundsW;  
    var mapHeight = isFloor ? mapScaleInfo.floorInfo.info.boundsH :mapScaleInfo.boundsH;  
    // console.log('mapWidth', mapWidth);
    
    var mapOrigin = isWar ? L.latLng(0, -80) : L.latLng(0, 0);
    var pixelToLatLngRatio = -1;
    var southWest = mapOrigin; // 左上角  
    var northEast = L.latLng((mapHeight - 70) * pixelToLatLngRatio, mapWidth * pixelToLatLngRatio); // 右下角  
    var bounds = L.latLngBounds(southWest, northEast);  
    // zoomAnimation: !0,
    // zoomAnimationThreshold: 4,
    // fadeAnimation: !0,
    // markerZoomAnimation: !0,
    // transform3DLimit: 8388608,
    // zoomSnap: 1,
    // zoomDelta: 1,
    // trackResize: !0

    map = L.map('MapContainer', {
        crs: L.CRS.Simple,
        attributionControl: false,
        zoomControl: false,
        maxBounds: bounds,
        maxBoundsViscosity: 1.0,
        minZoom: mapScaleInfo.minZoom,
        maxZoom: 8,
        preferCanvas: true,
        detectRetina: true,
        smoothSensitivity: 1,   // zoom speed. default is 1
        // zoomSnap: .1,
        wheelDebounceTime: 10,
        // 新增
        zoomAnimation: true,
        fadeAnimation: true,
        markerZoomAnimation: true,
        zoomAnimationThreshold: 2,
        transform3DLimit: 8388608,
        zoomSnap: 0.25,
        zoomDelta: 0.25,
        // zoomSnap: 0.1,
        // zoomDelta: 0.1,
        trackResize: !0
    }).setView([mapScaleInfo.initX,  mapScaleInfo.initY], mapScaleInfo.initZoom);
    let control = new L.Control.Zoomslider()
    map.addControl(control);
    window.pervInitX = mapScaleInfo.initX

    addLayer('map_db');
    if (queryMap[getQuery('map')]) {
        let getMap = queryMap[getQuery('map')];
        currMap = getMap[0];
        currLv = getMap[1];
        if (getQuery('map') === 'cgxg' || getQuery('map') === 'htjd') {
            $('.random-name').text(randomEvent[currMap].name)
            $('.btn-random').removeClass('hide')
        }
        changeMapLv(getMap);
    }
    if (getQuery('map').indexOf('dzc') !== -1) {
        enterWarMap();
    }
    currRegion = '行政辖区'
    map.on('zoomanim', function(e) {
        document.getElementById('MapContainer').classList.add('zooming');
        // console.log(e);
        // if (e.zoom < 4) {
        //     $('.curr-reigon').text('地图快速定位');
        // }
    });
    
    // 如果还需要在缩放时检查
    // map.on('zoomend', function() {
    //     const nearbyPoint = checkNearbyPoints();
    //     if (nearbyPoint) {
    //         // console.log(`缩放结束，用户位于"${nearbyPoint.name}"区域！距离: ${nearbyPoint.distance.toFixed(2)}`);
    //         // 在这里执行你需要的操作
    //     }
    // });
    
    // 地图移动时使用节流版本的检查函数
    // map.on('move', throttledCheckNearbyPoints);
    let nearbyPoint, prevNearbyPoint;
    // 移动结束时总是检查一次，确保不会漏掉最终位置
    map.on('moveend', function(e) {
        // console.log(e); 
        if (!isWar && mapScaleInfo?.floorInfo) {
         
            prevNearbyPoint = nearbyPoint;
            if (Number(currMap) === 4 && prevNearbyPoint?.name === nearbyPoint?.name) {
                isMoveFloor = false;
            }
            nearbyPoint = checkNearbyPoints();
            currRegion = nearbyPoint?.name;
            checkMoveFloor(nearbyPoint);
            
            // if (Number(currMap) === 4 && isFloor) {
            //     $('.curr-reigon').text(nearbyPoint?.name);
            //     $('.floor-tips .span1').text(nearbyPoint?.name)
            // }
        }
    });



    map.on('click', function(e) {
       // console.log(currClickMarker);
       if (currClickMarker) {
        currClickMarker.setIcon(currClickMarker.myIcon)
        $('.deploy-swiper').removeClass('show')
        document.getElementById('MapContainer').classList.remove('zooming');
       }
     
    });

    $('.leaflet-popup-pane').on('click', (e) => {
        // console.log(e);
        // console.log(currClickMarker);
        currClickMarker.closePopup();
        currClickMarker.setIcon(currClickMarker.myIcon)
        $('.deploy-swiper').removeClass('show')
        document.getElementById('MapContainer').classList.remove('zooming');
    })
    // initFloor();
    initNav();

    bindEvent();
    initQuickPosition();
};

// 边界数组转换
function filterPos (str, char1, char2, i) {
    var index = str.indexOf(char1);
    var index2 = str.indexOf(char2, index + 1);
    // // console.log(index, index2);
    
    if (index != -1 && index2 != -1) {
        return str.slice(index + 2, index2);
    }
    return str;
}


function drawBorderPub (color, border, border2) {
    var latlngs = [];
    var latlngs2 = [];
    var latlng = []

    for (let index = 0; index < border.length; index++) {
        const element = border[index];
        let x = filterPos(element, 'X', ',')
        let y = filterPos(element, 'Y', ',', 1)
        let pos = getMapPos(x, y)
        latlngs.push([pos.y, pos.x])
    }
    for (let index = 0; index < border2.length; index++) {
        const element = border2[index];
        let x = filterPos(element, 'X', ',')
        let y = filterPos(element, 'Y', ',', 1)
        let pos = getMapPos(x, y)
        latlngs2.push([pos.y, pos.x])
    }
    latlng.push(latlngs)
    latlng.push(latlngs2)
    const line = L.polygon(latlng, {color: color, fillColor: color, weight: 3, stroke: false}).addTo(map)
    borderList.push(line);
}
// 绘制边界
function drawBorder (color, border, isJd = false){ 

    var latlngs = [];
   

    for (let index = 0; index < border.length; index++) {
        const element = border[index];
        let x = filterPos(element, 'X', ',')
        let y = filterPos(element, 'Y', ',', 1)
        var pos = getMapPos(x, y)
        latlngs.push([pos.y, pos.x])
    }

    // 全地图边界
    let fourLatlng = [[0, mapScaleInfo.boundsW * -1],[-mapScaleInfo.boundsH, mapScaleInfo.boundsW * -1],[-mapScaleInfo.boundsH, -mapScaleInfo.boundsW * -1], [0, -mapScaleInfo.boundsW * -1]];
    var latlngs2 = [latlngs]
    

    // // 绘制且添加
    // transparent
    // if (isAttack) {
    //     var polyline = L.polyline(latlngs, {color: 'red'}).addTo(map);
    // } else {
    //     var polyline = L.polyline(latlngs, {color: 'green'}).addTo(map);
    // }
   
    
    if (isJd) {
        // const line = L.polygon(latlngs, {color: 'white', fillColor: 'white', weight: 3, stroke: false}).addTo(map)
        // const line2 = L.polygon(latlngs, {color: 'white', fillColor: 'white', weight: 3, stroke: false}).addTo(map)
        // const line3 = L.polygon(latlngs, {color: 'white', fillColor: 'white', weight: 3, stroke: false}).addTo(map)

        let colors
        if (window.occupy) {
            colors = 'white'
        } else {
            colors = window.viewChange ? 'red' : color
        }
        // console.log('colors', colors);
        
        const line = L.polygon(latlngs, {color: colors, fillColor: colors, weight: 3, stroke: false}).addTo(map)
        const line2 = L.polygon(latlngs, {color: colors, fillColor: colors, weight: 3, stroke: false}).addTo(map)
        const line3 = L.polygon(latlngs, {color: colors, fillColor: colors, weight: 3, stroke: false}).addTo(map)
        const line4 = L.polyline(latlngs, {color: colors}).addTo(map)
        // line.bringToFront();
        
        borderList.push(line);
        borderList.push(line2);
        borderList.push(line3);
        borderList.push(line4);
    } else {
        borderList.push(L.polyline(latlngs, {color}).addTo(map));
    }
   
    // var polygon = L.polygon(latlngs, { color: "transparent"}).addTo(map);

    // polygon.on('click', (e) => {
    //     // console.log(1, e);
    //     polyline.setStyle({color: 'red'});
    // })
}

function addLayer (mapName) {
    // console.log('currFloorIndex', currFloorIndex);
    
    // $('.leaflet-container').attr('class', `leaflet-container leaflet-touch leaflet-fade-anim leaflet-grab leaflet-touch-drag leaflet-touch-zoom map-${currMap}`)
    var mapWidth
    var mapHeight

    var minZoom, initZoom, initX, initY
    var mapOrigin
    var pixelToLatLngRatio;
    var southWest; // 左上角  
    if (window.occupy) {
        mapWidth = mapScaleInfo.boundsW_s
        mapHeight = mapScaleInfo.boundsH_s
        southWest = L.latLng(0, 0)
        pixelToLatLngRatio = -1
    } else if (isFloor) {
        // console.log(mapScaleInfo.floorInfo.info.floor[Number(currFloorIndex)]);
        
        mapWidth = mapScaleInfo.floorInfo.info.floor[Number(currFloorIndex)].boundsW
        mapHeight = mapScaleInfo.floorInfo.info.floor[Number(currFloorIndex)].boundsH
        // southWest = L.latLng(-25, 55)
        // pixelToLatLngRatio = -0.85
        // mapWidth = mapScaleInfo.boundsW
        // mapHeight = mapScaleInfo.boundsH
        southWest = L.latLng(mapScaleInfo.floorInfo.info.floor[Number(currFloorIndex)].latLngX, mapScaleInfo.floorInfo.info.floor[Number(currFloorIndex)].latLngY)
        pixelToLatLngRatio = mapScaleInfo.floorInfo.info.floor[Number(currFloorIndex)].pixelToLatLngRatio
    } else if (isWar) {
        mapWidth = mapScaleInfo.boundsW
        mapHeight = mapScaleInfo.boundsH
        southWest = L.latLng(0, -80)
        pixelToLatLngRatio = -1
    } else {
        mapWidth = mapScaleInfo.boundsW
        mapHeight = mapScaleInfo.boundsH
        southWest = L.latLng(0, 0)
        pixelToLatLngRatio = -1
    }
    var northEast = L.latLng((mapHeight - 70) * pixelToLatLngRatio, mapWidth * pixelToLatLngRatio); // 右下角  
    var href = '';
    if (!isFloor && mapScaleInfo.href) {
        href = mapScaleInfo.href
    
    } else if (isFloor && mapScaleInfo.floorInfo?.info?.href) {
        href = mapScaleInfo.floorInfo?.info?.href
    } else {
        href = '//game.gtimg.cn/images/dfm/cp/a20240729directory/img/'
    }

    if (window.occupy) {
        minZoom = mapScaleInfo.minZoom_s
        initZoom = mapScaleInfo.initZoom_s
        initX = mapScaleInfo.initX_s
        initY = mapScaleInfo.initY_s
    } else if (isFloor) {
        minZoom = mapScaleInfo.floorInfo.info.floor[Number(currFloorIndex)].minZoom
        initZoom =  mapScaleInfo.floorInfo.info.floor[Number(currFloorIndex)].initZoom
        initX = mapScaleInfo.floorInfo.info.floor[Number(currFloorIndex)].initX
        initY = mapScaleInfo.floorInfo.info.floor[Number(currFloorIndex)].initY;
        
    } else {
        minZoom = mapScaleInfo.minZoom
        initZoom =  mapScaleInfo.initZoom
        initX = mapScaleInfo.initX
        initY = mapScaleInfo.initY;
    }
    // console.log(minZoom, initZoom);
    var bounds = L.latLngBounds(southWest, northEast);  
    currLayer = L.tileLayer(href + `${mapName}/{z}_{x}_{y}.jpg`, {
        minZoom: minZoom,
        maxZoom: 8,
        maxNativeZoom: isFloor ? (mapScaleInfo.floorInfo.info?.maxZomm || 6) : 4,
        noWrap: false,
        attribution: '© OpenStreetMap contributors',
        bounds: bounds,
        errorTileUrl: href + `${mapName}/0_0_0.jpg`,
        tileSize: isFloor ? 512 : 256,
        zoomOffset: isFloor ? -1 : 0
    }).addTo(map);
    currLayer.name = mapName
    map.setMaxBounds(bounds)
    map.options.minZoom = minZoom;
    // console.log('视角定位', initX, mapScaleInfo);
    if (!isFloor) {
        $('.curr-reigon').text(`地图快速定位`)
    } else {
        $('.curr-reigon').text(`${currRegion}`)
    }
    
    if (mapName === 'map_qhz' && currWarType === 'mobile') {
        map.setView([window.occupy ? mapScaleInfo.initX_mobile_s : mapScaleInfo.initX, window.occupy ? mapScaleInfo.initY_mobile_s : mapScaleInfo.initY], initZoom)
    } else {
        map.setView([initX, initY], initZoom)
    }
   

    window.pervInitX = window.occupy ? mapScaleInfo.initX_s : mapScaleInfo.initX



    $.each(poiList, function () {
        this.remove();
    
    });
    poiList = [];
    let html = ''
    if (poiInfo && poiInfo.length > 0) {
        console.log('poiInfo', poiInfo);
        
        poiInfo?.forEach((item, index) => {
        if (item.name === '行政西楼' || item.name === '行政东楼') return;
        var myIcon =  L.divIcon({
            className: ` map-region-name`,
            html: `<div class="region-item"  data-x="${item.x}" data-y="${item.y}">${item.name}</div>`,
        })
        html+= `<div class="region-item region-item-${index}" data-x="${item.x}" data-y="${item.y}">${item.name}</div>`
        var pos = getMapPos(item.x, item.y)
        // // console.log(item.x, item.y);

    
        // regionList.append(`<div class="region-item region-item-${index}" data-x="${item.x}" data-y="${item.y}">${item.name}</div>`)
    
            poiList.push(L.marker([pos.y, pos.x], {icon: myIcon}).addTo(map))
        })
    }
        
    currRegion = poiInfo[0]?.name
    // 锚点定位
    regionList.html(html)
    initQuickPosition();
}

// 初始化楼层
function initFloor () {
    floorList.html('')
    floorList.append(`<div class="floor-item not-event act">大地图模式</div>`)
   
    // if (!mapScaleInfo.floor) return;
    // floorList.append(`<div class="floor-item not-event">大地图模式</div>`)
    floorTop = $('.map-floor-change-list')
    let html2 = '<div class="map-floor-item map-floor-item-normal btn_floor_mini_act" data-index="-1"></div>'
    mapScaleInfo.floor.forEach((item, index) => {
        let html = `<div class="floor-item floor-item-${index} ${currFloorIndex === index ? 'act' : ''}" data-index="${index}" data-floor="${item.floor_f}">${item.floor_f}</div>`
        html2 += `<div class="map-floor-item floor_${item.floor_f} ${currFloorIndex === index ? 'act' : ''}" data-index="${index}" data-floor="${item.floor_f}"></div>`
        floorList.append(html)
       
    })
    floorTop.html(html2)
    //console.log('initFloor', currRegion);
    
    $('.curr-reigon').text(currRegion);
    // 楼层切换按钮事件
    $('.map-floor-item').on('click', enterFloorMode);

    $('.floor-item').on('click', enterFloorMode)
    bindFloorEvent();
}

function bindFloorEvent () {
    $('.not-event').off('click').on('click', function (e) {
        console.log('bindFloorEvent');
        
        outFloorMode()
    })
}

// 进入分层模式通用方法
function enterFloorMode(e) {
    if ($(e.target).hasClass('not-event')) return;
    isFloor = true;
    outFloor = false;
    let index = Number($(e.target).attr('data-index'));
    let floor = $(e.target).attr('data-floor');
    let name = $(e.target).attr('data-name');
    currFloorIndex = index;
    // console.log('进入分层', currRegion);
    
    $('#MapContainer').addClass('floor')
    
    $('.map-floor .btn-floor').html(`<p>正在查看 <span class="color">${mapScaleInfo.floor[index].floor_f}层</span></p>`)
    $('.map-floor .btn-floor').attr('class', `btn-floor btn_floor floor-${mapScaleInfo.floor[index].floor_f}`)
    $('.floor-item').removeClass('act');
    $(e.target).addClass('act');
    $('.map-floor-change-ctn').addClass('show');
    $('.map-floor-item').removeClass('act');
    $(`.floor_${mapScaleInfo.floor[index].floor_f}`).addClass('act');
    $('.curr-reigon').text(mapScaleInfo.floor[currFloorIndex].floor_name);
    $('.floor-tips').html(`您正在查看<span class="span1">${mapScaleInfo.floor[currFloorIndex].floor_name}</span><span class="span2">返回大地图</span>`)
    initFloor();
    // 楼层切换按钮事件
    // $('.map-floor-item').off('click').on('click', (e) => {
    //     // console.log(e);
    //     let index = $(e.target).attr('data-index');
    //     currFloorIndex = index;
    //     $('.map-floor-item').removeClass('act');
    //     $(`.floor_${mapScaleInfo.floor[index].floor_f}`).addClass('act');
    //     $('.floor-item').removeClass('act');
    //     $(`.floor-item-${index}`).addClass('act');
    //     $('.map-floor .btn-floor').attr('class', `btn-floor btn_floor floor-${mapScaleInfo.floor[index].floor_f}`)
    // });

    // 退出楼层模式
    $('.floor-tips .span2').off('click').on('click', () => {
        outFloorMode();
    });
    saveMarker = Object.assign({}, visibleMarker);
    console.log(11111,saveMarker);
    const mapPath = isZj ? `${currMap + currLv}_s_${floor}` : `${currMap + currLv}_${floor}`;
    changeMapLv(mapPath);
    console.log('changeMapLv2', currMap, currLv);
    // if ($(e.target).hasClass('btn-floor')) {
    //     const nav = $(e.target).attr('data-nav');
    //     console.log(33333, saveMarker);
        
    //     visibleMarker = Object.assign({}, saveMarker);
    //     // console.log(22222, visibleMarker);
    //     // visibleMarker[name] = true;
    //     !visibleMarker[name] ? $(`.${nav}`).addClass('active'): $(`.${nav}`).removeClass('active')
    //     toggleVisible(name, currLeftNav);
    // }
    
    enterFloorSave();

    

}

// 进入楼层保留选项
function enterFloorSave () {
    // if (!isAll) return;
    let navType = {
        '保险柜': 'nav_bxx',
        '小保险箱': 'nav_xbxx',
        '服务器': 'nav_fwq',
        '电脑': 'nav_dn',
        '电脑机箱': 'nav_dnjx',
        '武器箱': 'nav_wqx',
        '大武器箱': 'nav_dwqx',
        '弹药箱': 'nav_dyx',
        '工具柜': 'nav_gjg',
        '大工具盒': 'nav_dgjx',
        '实验服': 'nav_yf_s',
        '衣服': 'nav_yf',
        '医疗包': 'nav_ylb',
        '医疗物资堆': 'nav_ylwzd',
        '旅行袋': 'nav_lxd',
        '手提箱': 'nav_stx',
        '储物柜': 'nav_cwg',
        '高级储物箱': 'nav_gjcwx',
        '抽屉柜': 'nav_ctg',
        '登山包': 'nav_dsb',
        '快递箱': 'nav_kdx',
        '航空储物箱': 'nav_hkcwx',
        '垃圾箱': 'nav_ljx',
        '水泥车': 'nav_snc',
        '野外物资箱': 'nav_ywwzx',
        '鸟窝': 'nav_nw',
        '藏匿物': 'nav_cnw',
        '高级旅行箱': 'nav_xlx',
        '出生点': 'nav_csd',
        '付费撤离点': 'nav_ffcld',
        '常规撤离点': 'nav_cgcld',
        '概率撤离点': 'nav_sjcld',
        '条件撤离点': 'nav_tjcld',
        '电梯撤离点': 'nav_dtcld',
        '滑索撤离点': 'nav_hscld',
        '火箭撤离点': 'nav_htcld',
        '行动撤离点': 'nav_xdcld',
        '首领': 'nav_boss',
        
    }
    console.log('enterFloorSave', saveMarker);
    for (const key in saveMarker) {
        if (saveMarker[key]) {
            console.log(1111, key);
            // console.log(navType[key]);
            
            !visibleMarker[key] ? $(`.nav-list-${navType[key]}`).addClass('active'): $(`.${navType[key]}`).removeClass('active')
            toggleVisible(key, currLeftNav);
        }
    }
    visibleMarker = Object.assign({}, saveMarker);
    // !visibleMarker[name] ? $(`.${nav}`).addClass('active'): $(`.${nav}`).removeClass('active')
    // toggleVisible(name, currLeftNav);
}

function outFloorMode () {
    isFloor = false;
    $('#MapContainer').removeClass('floor')
    currFloorIndex = -1;
    $('.not-event').addClass('act');
    $('.map-floor-change-ctn').removeClass('show');
    $('.map-floor .btn-floor').html(`<p>查看地图分层</p>`)
    $('.map-floor .btn-floor').attr('class', 'btn-floor btn_floor')
    floorList.html('')
    floorList.append(`<div class="floor-item not-event">大地图模式</div>`)
    $('.floor-tips').html(`<p> 您正在查看<span class="span1">${currRegion}</span></p>`)
    changeMapLv(`${currMap + currLv}`);
    enterFloorSave();
    outFloor = true;
    console.log('changeMapLv3', currMap, currLv);
    // console.log('初始化currRegion2', currRegion);
    
}


// 重置全选
function resetAll (type) {
    console.log('resetAll', currLeftNav,type);
    
    if (currLeftNav == 0) {
        if (listIsAll[0]) {
            for (const key in listIsAll) {
                if (Object.hasOwnProperty.call(listIsAll, key)) {
                    listIsAll[key] = false
                }
            }
            console.log('走1');
            
        } else {
            for (const key in listIsAll) {
                if (Object.hasOwnProperty.call(listIsAll, key)) {
                    listIsAll[key] = type === 'none' ? false : true
                }
            }
        }
       
    } else if (type === 'none') {
        // console.log('走这里');
        for (const key in listIsAll) {
            if (Object.hasOwnProperty.call(listIsAll, key)) {
                listIsAll[key] = false
            }
        }
    } else {
        // listIsAll[currLeftNav] = type === 'none' ? false: true;
        listIsAll[currLeftNav] = listIsAll[currLeftNav] ? false: true;
    }
    
    if (listIsAll[1] && listIsAll[2] && listIsAll[3] && listIsAll[4] && listIsAll[5]) {
        listIsAll[0] = true;
    } else if (!listIsAll[1] || !listIsAll[2] || !listIsAll[3] || !listIsAll[4] || !listIsAll[5]) {
        listIsAll[0] = false;
    }
   
}

var initNav = function () {
    currLeftNav = 0;
    var navLeft = $('.nav-options');
    navLeft.html('')
    var navList;
    if (isWar) {
        navList = allNavList.typeList;
    } else {
        navList = allNavList
    }
    

    
    allNavList.forEach(function (item, index) {
        navLeft.append(`<div class="nav-option-item nav-option-item-${index} ${currLeftNav === index ? 'active': ''}" style="${item.title === '行动接取站' ? 'display: none' : ''}" data-index="${index}">${item.title}</div>`)
       
    })

    var navOptItem = $('.nav-option-item')
    // 选择类型
    navOptItem.on('click', function (e) {
        var index = $(e.target).attr('data-index');
        currLeftNav = index;
        navOptItem.removeClass('active')
        $(`.nav-option-item-${index}`).addClass('active')
        // console.log($(e.target).attr('data-index'));
        if (Number(index) === 0) {
            renderNavTypeList(allNavList[0].typeList, 0)
        } else {
            // console.log('navTypeList[index]', navTypeList, index, navTypeList[index]);
            
            renderNavTypeList(navTypeList[index].typeList, index)
        }

        if (listIsAll[currLeftNav]) {
            $('.btn-choose-all-icon').attr('class', 'img_all_open btn-choose-all-icon')
        } else {
            $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon')
        }
        
        bindOptionEvent();

        // if (isWar) {
        //     warInit(currWarMap, currWarType);
        // }

    })
    
    renderNavTypeList(allNavList[0].typeList, 0)
    

        
    regionList.html('')
    selectRegion.forEach(function (item, index) {
        regionList.append(`<div class="region-item region-item-${index}" data-x="${item.x}" data-y="${item.y}">${item.name}</div>`)
    })
}

var renderNavTypeList = function (list, navIndex = 0) {
    
    // 定义分类容器
    const categories = {
        wz: { title: '物资点', html: '' },
        csd: { title: '出生点', html: '' },
        cld: { title: '撤离点', html: '' },
        sl: { title: '首领', html: '' },
        jd: { title: '据点', html: '' },
        jdbsd: { title: '基地部署点', html: '' },
        zj: { title: '载具', html: '' },
        zjbjz: { title: '载具补给站', html: '' },
        gddyx: { title: '固定弹药箱', html: '' },
        gdwq: { title: '固定武器', html: '' },
        zz: { title: '装置', html: '' }
    };

    // 分类处理函数
    function addToCategory(item, index, category) {
        // 获取该item.name对应的额外类名
        const extraClass = nameClassMap[item.name] || '';
        
        categories[category].html += `
            <div class="nav-list-item nav-list-item-${index} nav-list-${item.icon} ${visibleMarker[item.name] ? 'active': ''} ${extraClass} ${item.num === 0? 'hide': ''}" data-index="${index}" data-icon="${item.icon}" data-name="${item.name}">
                <div class="wz-bg">
                    <div class="wz-icon img_${item.icon} "></div>
                    <div class="wz-num ${item.num === 1? 'hides': ''}">${item.num}</div>
                </div>
                <div class="wz-name">${item.name}</div>
            </div>`;
        }

    // 遍历列表并分类
    list.forEach((item, index) => {
        if (item.name === '行动接取站' || item.name === '高价值接取站') return;
        
        if (item.name.indexOf('撤离点') !== -1) {
            addToCategory(item, index, 'cld');
        } else if (item.name.indexOf('出生点') !== -1) {
            addToCategory(item, index, 'csd');
        } else if (item.name.indexOf('首领') !== -1) {
            addToCategory(item, index, 'sl');
        } else if (item.name.indexOf('基地') > -1) {
            addToCategory(item, index, 'jdbsd');
        // } else if (item.name.indexOf('据点') > -1 && !window.occupy) {
        } else if (item.name.indexOf('据点') > -1) {
            addToCategory(item, index, 'jd');
        } else if (isWar && (item.name.indexOf('车') > -1 || item.name.indexOf('舟') > -1 || item.name.indexOf('轮式') > -1)) {
            addToCategory(item, index, 'zj');
        } else if (item.name.indexOf('载具补给站') > -1) {
            addToCategory(item, index, 'zjbjz');
        } else if (item.name.indexOf('固定弹药箱') > -1) {
            addToCategory(item, index, 'gddyx');
        } else if (isWar && (item.name.indexOf('枪') > -1 || item.name.indexOf('炮') > -1)) {
            addToCategory(item, index, 'gdwq');
        } else if (item.name.indexOf('滑索') > -1 || item.name.indexOf('电梯') > -1) {
            addToCategory(item, index, 'zz');
        } else {
            addToCategory(item, index, 'wz');
        }
        !typeListInit &&  (visibleMarker[item.name] = false)
    });

    // 按指定顺序生成最终 HTML
    let finalHtml = '';
    Object.keys(categories).forEach(key => {
        // console.log(key);
        
        if (categories[key].html) {
            
            finalHtml += `<div class="nav-type-item nav-${key}"><div class="fgx top0 type-item-fgx" data-nav="${key}">${categories[key].title}</div>${categories[key].html}</div>`;
        }
    });
    if (isWar) {
        navTypyList.addClass('war')
        $('.nav-ctn').addClass('img_nav_bg_war')
        navTypyList.removeClass('normal')

    } else {
        navTypyList.addClass('normal')
        navTypyList.removeClass('war')
        $('.nav-ctn').removeClass('img_nav_bg_war')
    }
    navTypyList.html(finalHtml);

    typeListInit = true;
    visibleMarker2[navIndex].isInit = true;

    $('.type-item-fgx').on('click', (e) => {
        const type = $(e.target).attr('data-nav');
        $(`.nav-${type}`).attr('class').indexOf('close') > -1 ? $(`.nav-${type}`).removeClass('close') :  $(`.nav-${type}`).addClass('close')
    })
}

function toastTips () {
    $('.m-toast').show();
    setTimeout(() => {
        $('.m-toast').hide();
    }, 800)
}


function fuzzyMatch(text, pattern) {
    // 将模糊词转换为正则表达式
    const regex = new RegExp(pattern.split('').join('.*'), 'i');
    return regex.test(text);
}

var mapSelectCtn = $('.map-select')
var selectCtn = $('.select-ctn');
// 搜索
function selectmarker(name) {
    if (name === '') {
        mapSelectCtn.removeClass('show');
        return;
    };
    // console.log('name', name, allNavList);
    var markerList = []
    var html = '';
    for (let index = 0; index < allNavList[0].typeList.length; index++) {
        const element = allNavList[0].typeList[index];
        // console.log(element);
        fuzzyMatch(element.name, name) && markerList.push(element)
    }
    
    // console.log('markerList', markerList);
    markerList.length && markerList.forEach(function (item, index) {
        // 获取该item.name对应的额外类名
        const extraClass = nameClassMap[item.name] || '';
        
        // // console.log(visibleMarker[item.name], item.name);
        html+=`
        <div class="nav-list-item nav-list-item-${index} nav-list-${item.icon} ${visibleMarker[item.name] ? 'active': ''} ${extraClass} ${item.num === 0? 'hide': ''}" data-index="${index}" data-icon="${item.icon}" data-name="${item.name}">
            <div class="wz-bg">
                <div class="wz-icon img_${item.icon}"></div>
                <div class="wz-num">${item.num}</div>
            </div>
            <div class="wz-name">${item.name}</div>
        </div>`

    })
    // navTypyList.html(html)
    selectCtn.html(html)
    mapSelectCtn.addClass('show');

    bindOptionEvent();
}

function bindOptionEvent () {
    var NavListItem = $('.nav-list-item')

    
    // 选择标签
    NavListItem.on('click', function (e) {
        var index = $(e.target).attr('data-index');
        var name = $(e.target).attr('data-name');
        var icon = $(e.target).attr('data-icon')
        // console.log('index', $(e.target).attr('data-icon'));
        NavCliciIndex++;
        currNavIcon = name;
        // if (isWar) {
        //     $(this).attr('class').indexOf('active') ===  -1 ?  $(this).addClass('active'): $(this).removeClass('active')
        // } else {
        //     !visibleMarker[name] ? $(this).addClass('active'): $(this).removeClass('active')
        //     toggleVisible(name, currLeftNav);
        // }
        !visibleMarker[name] ? $(this).addClass('active'): $(this).removeClass('active')
        toggleVisible(name, currLeftNav);
        saveMarker = Object.assign({}, visibleMarker);
        
        let chooseNum = $('.nav-type-list').find('.active').length;

        // console.log('chooseNum', chooseNum, currLeftNav, navTypeList, navTypeList[currLeftNav].typeList);
        
        if (isFloor && chooseNum === navTypeList) {
            $('.btn-choose-all-icon').attr('class', 'img_all_open btn-choose-all-icon')
        } else if (!isFloor && chooseNum === navTypeList[currLeftNav].typeList.length) {
            $('.btn-choose-all-icon').attr('class', 'img_all_open btn-choose-all-icon')
        } else {
            if (listIsAll[currLeftNav]) {
                listIsAll[currLeftNav] = false;
                listIsAll[0] = false;
                $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon')
            } else {
                $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon')
            }
        }
        // console.log('点击');
    })
}

// 事件
var bindEvent = function () {
    // 导航栏状态
    var navState = true;
    var navCtn = $('.nav-ctn')
    var navOptItem = $('.nav-option-item')
    var btnNavState = $('.btn-nav-state');
    var isTimer;
    // 搜索地区



    $('.choose-all').on('click', function () {
        // resetNav();
        isAll = !isAll;
        
        resetAll()
        // console.log('allNavList',listIsAll[currLeftNav], allNavList);
        
        if (isAll) {
            toggleVisible(`${currLeftNav}_all`, currLeftNav);
            $('.btn-choose-all-icon').attr('class', 'img_all_open btn-choose-all-icon')
        } else {
            toggleVisible(`${currLeftNav}_none`, currLeftNav);
            $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon')
        }
        // renderNavTypeList(navList[0].typeList)
        if (Number(currLeftNav) === 0) {
            // console.log('进入', allNavList[0].typeList);
            
            renderNavTypeList(isFloor ? navTypeList : allNavList[0].typeList, 0)
        } else {
            renderNavTypeList(navTypeList[currLeftNav].typeList, currLeftNav)
        }

        bindOptionEvent();

    })

    $('.reset-choose').on('click', function () {
        resetAll('none')
        toggleVisible(`none`, currLeftNav);
            $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon')
        // renderNavTypeList(navList[0].typeList)
        if (isFloor) {
            renderNavTypeList(navTypeList, 0)
        } else if (Number(currLeftNav) === 0) {
            renderNavTypeList(allNavList[0].typeList, 0)
        } else {
            renderNavTypeList(navTypeList[currLeftNav].typeList, currLeftNav)
        }

        bindOptionEvent();
    })

    // 选择类型
    navOptItem.on('click', function (e) {
        var index = $(e.target).attr('data-index');
        currLeftNav = index;
        navOptItem.removeClass('active')
        $(`.nav-option-item-${index}`).addClass('active')
        // console.log($(e.target).attr('data-index'));
        if (isFloor) {
            renderNavTypeList(navTypeList, 0)
        } else if (Number(index) === 0) {
            renderNavTypeList(allNavList[0].typeList, 0)
        } else {
            renderNavTypeList(navTypeList[index].typeList, index)
        }

        if (listIsAll[currLeftNav]) {
            $('.btn-choose-all-icon').attr('class', 'img_all_open btn-choose-all-icon')
        } else {
            $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon')
        }

        bindOptionEvent();

    })

    bindOptionEvent();
    bindFloorEvent();

    // 打开关闭导航
    btnNavState.on('click', function () {
        navState = !navState;
        if (navState) {
            navCtn.attr('class', 'open img_nav_bg nav-ctn')
        } else {
            navCtn.attr('class', 'close img_nav_bg nav-ctn')
        }
    })



    // 切换地图移入事件
    let currMoveMap, currMoveWarMap,currWarMap_s;
    dom_changeMapBtn.on('mouseenter', function () {
        dom_mapList.css({height: '2.55rem', width: '1.2rem'});
        dom_changeMapBtn.css('height', '1rem')
        dom_changeMapBtn.addClass('hover')
    })

    dom_changeMapBtn.on('mouseleave', function () {
        // dom_mapList.css('height', '0px');
        // console.log('移出');
        // console.log('currMoveWarMap', currMoveWarMap);
        if (!isMoveMapList) {
            dom_mapList.css('height', '0px');
            dom_mapList.css('width', '1.2rem');
            dom_changeMapBtn.css('height', '0.3rem')
            dom_changeMapBtn.removeClass('hover')
            dom_mapList.attr('class', `map-list-ctn hover_0`)
        }
    })
    let mapItem =  $('.map-item');

    dom_mapList.on('mouseover', function (e) {
        var index = $(e.target).attr('data-index')
        var warMap = $(e.target).attr('data-map')
        // console.log(index, warMap);

        if (index || index == 0) {
            currMoveMap = index;
            currMoveWarMap = warMap;
            console.log('赋值1',currMoveMap );
            
            // currMap = index;
            // currWarMap_s = warMap;
            // currWarMap = warMap
            dom_mapList.attr('class', `map-list-ctn hover_${index}`)
            
        }

        if (warMap) {
            mapItem.removeClass('action')
            // currWarMap = warMap;
            currMoveWarMap = warMap;
            // console.log('赋值2',currMoveWarMap );
            $(e.target).addClass('action')
        }
        dom_mapList.css('width', '4rem');
        isMoveMapList = true;
    })

    dom_mapList.on('mouseleave', function () {
        dom_mapList.css('height', '0px');
        dom_mapList.css('width', '1.2rem');
        dom_changeMapBtn.css('height', '0.3rem')
        dom_changeMapBtn.removeClass('hover')
        dom_mapList.attr('class', `map-list-ctn hover_0`)
        isMoveMapList = false;
        // // console.log('这里移入啥');
        
        // currWarMap_s = currMoveMap;
        // // console.log('赋值4',currWarMap_s );
        $('.map-lv-item').removeClass('action')
        mapItem.removeClass('action')
    })

    $('.map-lv-item').on('mouseover', function (e) {
        var lv = $(e.target).attr('data-lv')
        var type = $(e.target).attr('data-type')
        
        if (lv && Number(lv) > 0) {
            // console.log(lv);
            $('.btn-type-change').attr('class').indexOf('top') === -1 && $('.btn-type-change').addClass('top')
        } else {
            $('.btn-type-change').removeClass('top')
        }

        if (type) {
            $('.map-lv-item').removeClass('action')
            currMoveWarMap = currMoveWarMap;
            // console.log('赋值3',currMoveWarMap );
            currWarType = type;
            $(e.target).addClass('action')
        }
    })

    // 选择难度等级
    $('.map-lv-item').on('click', function (e) {
        var lv = $(e.target).attr('data-lv')
        
        var type = $(e.target).attr('data-type')
        // console.log('isZj', isZj);
        isAll = false;
        currMap = currMoveMap;
        currWarMap = currMoveWarMap;
        if (isFloor) {
            resetFloor();
            // outFloorMode(); // 重复执行
        }

        chooseItemLvName = $(e.target).text();
        if (isWar) {
            window.warLv = 0
            $('.lv-change-tips').text(`区域${warLvText[window.warLv]}`)
            
            resetAll('none')
            toggleVisible(`none`, currLeftNav);
            changeWarMap(currWarMap, type)
    
            
            // initNav();
            // bindOptionEvent();
        } else if (isZj && randomEvent[currMap]) {
            changeMapLv(currMap + lv + '_s');
            
        } else {
            changeMapLv(currMap + lv);
        }
        console.log('changeMapLv4', currMap, currLv);

        
        console.log('选择难度后', currMap, currLv, currMoveMap);
        if (randomEvent[currMap]) {
            $('.random-name').text(randomEvent[currMap].name)
            $('.btn-random').removeClass('hide')
        } else {
            $('.random-name').text('暂无')
            $('.btn-random').addClass('hide')
        }

        if (currMap + currLv !== currMap + lv) {
            // console.log('走了这里');
            
            currLv = lv
            currWarType = type
            if (window.pervInitX === window.occupy ? mapScaleInfo.initX_s : mapScaleInfo.initX) return;
            map.flyTo([window.occupy ? mapScaleInfo.initX_s : mapScaleInfo.initX,  window.occupy ? mapScaleInfo.initY_s : mapScaleInfo.initY], window.occupy ? mapScaleInfo.initZoom_s : mapScaleInfo.initZoom)
            window.pervInitX = window.occupy ? mapScaleInfo.initX_s : mapScaleInfo.initX
        }



        dom_mapList.css('height', '0px');
        dom_mapList.css('width', '1.2rem');
        dom_changeMapBtn.css('height', '0.3rem')
        dom_changeMapBtn.removeClass('hover')
        dom_mapList.attr('class', `map-list-ctn hover_0`)
        isMoveMapList = false;
        // $('.btn-random').removeClass('open')
        if (!isWar) {
            resetAll('none')
            toggleVisible(`none`, currLeftNav);
        }

    })

    // 坠机事件
    $('.btn-random').on('click', function () {
        // 切换随机状态
        isZj = !isZj;
        // currMap = currMoveMap;
        isFloor && resetFloor();
        
        // 确定目标地图层级
        let targetMapLv;
        if (isZj) {
            // 随机模式下的地图层级
            targetMapLv = currMap + currLv + '_s';
        } else {
            // 常规模式下的地图层级
            targetMapLv = currMap + currLv;
        }
        
        
        // 切换地图层级
        changeMapLv(targetMapLv);
        
        // 如果当前地图与目标不同，则飞到初始位置
        // if (currMap + currLv !== targetMapLv) {
        //     map.flyTo([mapScaleInfo.initX, mapScaleInfo.initY], mapScaleInfo.initZoom);
        // }
        
        // 重置标记和可见性
        resetAll('none');
        toggleVisible(`none`, currLeftNav);
    });
    // 打开日志
    $('.btn-log').on('click', function () {
        $('.log-pop').fadeIn();
        // toastTips();
    })

    // 关闭日志弹窗
    $('.btn-close-pop').on('click', function () {
        $('.log-pop').fadeOut();
    })

    // 打开教程
    $('.btn-tutorial').on('click', function () {
        toastTips();
    })

    // 反馈
    $('.btn-feedback').on('click', function () {
        window.open('https://wj.qq.com/s2/15023838/70c7/')
    })

    $('.bottom-bar-ctn').on('click', function (e) {
        // toastTips();
        e.stopPropagation();
    })


    $('.btn-share').on('click', function () {
        $('.bottom-bar').fadeIn();
        $('.bottom-bar').addClass('show')
    })
    $('.bottom-bar').on('click', function() {
        $('.bottom-bar').removeClass('show')
        $('.bottom-bar').fadeOut();
    })


    // 搜索
    $('.select-iput').on('input',  debounce(function (e) {
        var name = $(e.target).val()
        selectmarker(name)
    }, 500));

    // 关闭搜索
    $('.btn-close-select').on('click', function () {
        $('.select-iput').val('')
        $('.map-select').removeClass('show')
        if (Number(currLeftNav) === 0) {
            renderNavTypeList(allNavList[0].typeList, 0)
        } else {
            renderNavTypeList(navTypeList[currLeftNav].typeList, currLeftNav)
        }
        bindOptionEvent();
        PTTSendClick && PTTSendClick('btn', 'close', '关闭弹窗');
    })

    $('.lj').on('click', () => {
        copyToClipboard(window.location.href)
        $('.toast-copy').css('display', 'block');
        setTimeout(() => {
            $('.toast-copy').css('display', 'none');
        }, 800)
    })


    navTypyList.on('scroll', function (e) {

        var aScrollHeight = document.querySelector('.nav-type-list').scrollHeight - document.querySelector('.nav-type-list').clientHeight;
        if ((aScrollHeight - navTypyList.scrollTop() >= -10 && aScrollHeight - navTypyList.scrollTop() <= 10)) {
            navTypyList.addClass('bot')
        } else {
            navTypyList.removeClass('bot')
        }
    })

    var regionName = $('.map-region-name')
    window.addEventListener('zoom', function (e) {
        regionName.attr('class', `map-region-name lv-${e.detail}`)
        // // console.log(Number(e.detail), e.detail);
        // if (Number(e.detail) >= 2) {
        //     // console.log(1111);
        //     mapFolder = '5/';
        //     currLayer.setUrl(`../../img/map_db/5/{z}_{x}_{y}.jpg`);
          
        // } else {
        //     currLayer.setUrl(`../../img/map_db/0_4/{z}_{x}_{y}.jpg`);
        // }
    })

    // 进攻方视角
    $('.view-change1').on('click', () => {
        if (!window.viewChange) {
            window.viewChange = true;
            window.isViewChange = true;
            $('.btn-view-change').addClass('g')
            $('.btn-view-change').removeClass('f')
            $('.nav-option-ctn').addClass('g');
            $('.nav-option-ctn').removeClass('f');
            $('.war-lv-change-list').attr('class', 'war-lv-change-list g')
           
            warInit(currWarMap, currWarType, true);
            viewChangeToMap();


            setTimeout(() => {
                window.isViewChange = false;
            }, 800);
        }
    })

    // 防守方视角
    $('.view-change2').on('click', () => {
        if (window.viewChange) {
            window.viewChange = false;
            window.isViewChange = true;
            $('.btn-view-change').addClass('f')
            $('.btn-view-change').removeClass('g')
            $('.nav-option-ctn').addClass('f');
            $('.nav-option-ctn').removeClass('g');
            $('.war-lv-change-list').attr('class', 'war-lv-change-list f')


            warInit(currWarMap, currWarType, true);
            // // console.log(cacheMarker);
            viewChangeToMap();
            setTimeout(() => {
                window.isViewChange = false;
            }, 800);

        }
    })

    $('.btn-type-change').on('click', (e) => {
        var type = $(e.target).attr('data-type')
        type === 'occupy' ? (window.occupy = true) : (window.occupy = false)
        // console.log('window.occupy', window.occupy, currMoveWarMap);
        currWarMap = currMoveWarMap;
        // window.occupy = !window.occupy;
        window.warLv = 0
        console.log('changeWarMap', currWarMap, currWarType);
        $('.lv-change-tips').text(`区域${warLvText[window.warLv]}`)
        if (window.occupy) {
            $('.btn-type-change').addClass('open')
            // $('.war-lv-change-ctn').addClass('show')
            $('.war-lv-change-ctn').removeClass('show')
        } else {
            $('.btn-type-change').removeClass('open')
            $('.war-lv-change-ctn').addClass('show')
         
        }
        // console.log('currWarMap', currWarMap, currWarType);
        
        changeWarMap(currWarMap, currWarType);
        // initNav();
        // bindOptionEvent();
        // warInit(currWarMap, currWarType);
    })

    $('.lv-change-prev').on('click', () => {
        if (window.isLvChange) return;
        if (window.warLv === 0) return;
        window.isLvChange = true;
        window.warLv--;
        changeWarMap(currWarMap, currWarType);
        // initNav();
        // bindOptionEvent();
        $('.lv-change-tips').text(`区域${warLvText[window.warLv]}`)
        $('.deploy-swiper').removeClass('show')
        map.flyTo([window[currWarMap].info.sectorInit[window.warLv].initX,  window[currWarMap].info.sectorInit[window.warLv].initY], window[currWarMap].info.sectorInit[window.warLv].initZoom)
        listIsAll[currLeftNav] = false;
        $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon')
        setTimeout(() => {
            window.isLvChange = false;
        }, 500)
    })

    $('.lv-change-next').on('click', () => {
        if (window.isLvChange) return;
        if (window.warLv === window[currWarMap].info.sector - 1) return;
        window.isLvChange = true;
        window.warLv++;
        changeWarMap(currWarMap, currWarType);
        // initNav();
        // bindOptionEvent();
        $('.lv-change-tips').text(`区域${warLvText[window.warLv]}`)
        $('.deploy-swiper').removeClass('show')
        // console.log('currWarMap', currWarMap);

        map.flyTo([window[currWarMap].info.sectorInit[window.warLv].initX,  window[currWarMap].info.sectorInit[window.warLv].initY], window[currWarMap].info.sectorInit[window.warLv].initZoom)
        listIsAll[currLeftNav] = false;
        $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon')
        setTimeout(() => {
            window.isLvChange = false;
        }, 500)
    })

    $('.btn-swiper-prev').on('click', () => {
        window.warSwiper.slidePrev();
    })

    $('.btn-swiper-next').on('click', () => {
        window.warSwiper.slideNext();
    })

    $('.war-change-text').on('click', () => {
        if (isWar) return;
        enterWarMap();
    })

    $('.map-change-text').on('click', () => {
        if (!isWar) return;
        isWar = false;
        
        $('#MapContainer').addClass('map')
        window.occupy = false;
        window.viewChange = true;
        $('.btn-view-change').addClass('g')
        $('.btn-view-change').removeClass('f')
        $('.nav-option-ctn').addClass('g');
        $('.nav-option-ctn').removeClass('f');
        $('.war-lv-change-list').attr('class', 'war-lv-change-list g')
        $('.map-floor').css('display', 'block');
        $('.random-ctn').css('display', 'flex');

        dom_changeMapBtn.removeClass('war')
        $('.btn-war-change').addClass('fh')
        $('.btn-war-change').removeClass('war')
        $('.nav-ctn').removeClass('img_nav_bg_war')
        $('.btn-view-change').attr('class', 'btn-view-change')
        resetAll('none');
        toggleVisible(`none`, currLeftNav);
        changeMapLv('00');
        warRemove();
        currMap = '0';
        currLv = '0'
        initNav();
        bindOptionEvent();
        initQuickPosition();
        $('.war-lv-change-ctn').removeClass('show')
        $('.select-region-ctn').css('display', 'block')
        $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon');
    })



    // 处理楼层切换的通用函数
    function handleFloorChange(index) {
        $('.map-floor-item, .floor-item').removeClass('act');
        $(`.floor_${mapScaleInfo.floor[index].floor_f}`).addClass('act');
        $(`.floor-item-${index}`).addClass('act');
        $('.floor-tips .span1').text(mapScaleInfo.floor[index].floor_name);
        changeMapLv(`${currMap}${currLv}_${mapScaleInfo.floor[index].floor_f}`);
        console.log('初始化后', currMap, currLv);
        console.log('changeMapLv5', currMap, currLv);
        enterFloorSave();
    }

    // 上一层
    $('.floor-change-prev').on('click', () => {
        if (currFloorIndex > 0) {
            currFloorIndex--;
            handleFloorChange(currFloorIndex);
            $('.map-floor .btn-floor').html(`<p>正在查看 <span class="color">${mapScaleInfo.floor[currFloorIndex].floor_f}层</span></p>`)
            $('.map-floor .btn-floor').attr('class', `btn-floor btn_floor floor-${mapScaleInfo.floor[currFloorIndex].floor_f}`)
        } else if (currFloorIndex === -1) {
            isFloor = true;
            outFloor = false;
            saveMarker = Object.assign({}, visibleMarker);
            currFloorIndex = 0
            handleFloorChange(currFloorIndex);
            $('.floor-tips').html(`您正在查看<span class="span1">${mapScaleInfo.floor[currFloorIndex].floor_name}</span><span class="span2">返回大地图</span>`)
            $('.map-floor .btn-floor').attr('class', `btn-floor btn_floor floor-${mapScaleInfo.floor[currFloorIndex].floor_f}`)
           
        } else {
            currFloorIndex = mapScaleInfo.floor.length - 1
            handleFloorChange(currFloorIndex);
            $('.floor-tips').html(`您正在查看<span class="span1">${mapScaleInfo.floor[currFloorIndex].floor_name}</span><span class="span2">返回大地图</span>`)
            $('.map-floor .btn-floor').attr('class', `btn-floor btn_floor floor-${mapScaleInfo.floor[currFloorIndex].floor_f}`)
        }
        $('.floor-tips .span2').off('click').on('click', () => {
            outFloorMode();
        });
    });

    // 下一层
    $('.floor-change-next').on('click', () => {
        if (currFloorIndex === -1) {
            isFloor = true;
            outFloor = false;
            saveMarker = Object.assign({}, visibleMarker);
            // console.log('进入前', saveMarker);
            
        }

        if (currFloorIndex < mapScaleInfo.floor.length - 1) {
            currFloorIndex++;
            $('.floor-tips').html(`您正在查看<span class="span1">${mapScaleInfo.floor[currFloorIndex].floor_name}</span><span class="span2">返回大地图</span>`)
            handleFloorChange(currFloorIndex);
            $('.map-floor .btn-floor').html(`<p>正在查看 <span class="color">${mapScaleInfo.floor[currFloorIndex].floor_f}层</span></p>`)
            $('.map-floor .btn-floor').attr('class', `btn-floor btn_floor floor-${mapScaleInfo.floor[currFloorIndex].floor_f}`)
            
        } else {
            currFloorIndex = 0
            isFloor = true;
            outFloor = false;
            saveMarker = Object.assign({}, visibleMarker);
            handleFloorChange(currFloorIndex);
            $('.floor-tips').html(`您正在查看<span class="span1">${mapScaleInfo.floor[currFloorIndex].floor_name}</span><span class="span2">返回大地图</span>`)
            $('.map-floor .btn-floor').attr('class', `btn-floor btn_floor floor-${mapScaleInfo.floor[currFloorIndex].floor_f}`)
            
        }
        $('.floor-tips .span2').off('click').on('click', () => {
            // console.log(1111);
            
            outFloorMode();
        });
        $('#MapContainer').addClass('floor')
        $('.floor-tips').css('display', 'block');
    });

}

// 快速定位事件绑定
function initQuickPosition () {
    let isTimer;
    $('.region-item').off('click').on('click', function (e) {
        $('#MapContainer').removeClass('map')
        clearTimeout(isTimer);
        var x = $(e.target).attr('data-x');
        var y = $(e.target).attr('data-y');
        var pos = getMapPos(x, y)
        if (isFloor) {
            outFloorMode(); 
                setTimeout(() => {
                    map.flyTo([pos.y * 2, pos.x * 2], 5)
                }, 500)
            
        } else {
            map.flyTo([pos.y, pos.x], 5)
        }
        // console.log(pos);
        isTimer = setTimeout(() => {
            $('#MapContainer').addClass('map')
        }, 2000)
        currRegion = $(e.target).text();
        $('.curr-reigon').text(currRegion);
        if (findFloor(currRegion)) {
           // console.log('进入楼层', currFloorIndex);
           
            initFloor();
            // isFloor = true;
            // $('#MapContainer').addClass('floor')
            $('.not-event').addClass('act');
            $('.map-floor-change-ctn').addClass('show');
            $('.map-floor .btn-floor').html(`<p>请选择楼层</p>`)
            $('.floor-tips').html(`<p> 您正在查看<span class="span1">${currRegion}</span></p>`)
        } else {
            isFloor = false;
            currFloorIndex = -1;
            // console.log('进入这里2', currFloorIndex);
            
            floorList.html('')
            floorList.append(`<div class="floor-item not-event">大地图模式</div>`)
            $('.map-floor-change-ctn').removeClass('show');
        }
    })
}


// 进入大战场
function enterWarMap () { 
    isWar = true;
    resetFloor();
    $('#MapContainer').removeClass('map')
    dom_changeMapBtn.addClass('war')
    $('.btn-war-change').addClass('war')
    $('.nav-ctn').addClass('img_nav_bg_war')
    $('.btn-war-change').removeClass('fh')
    $('.btn-view-change').addClass('g')
    $('.map-floor').css('display', 'none');
    $('.random-ctn').css('display', 'none');
    // changeWarMap('jq', 'pc');
    currWarMap = 'pc';
    currWarType = 'pc';
    if (getQuery('map').indexOf('dzc') !== -1) {
        // 从第4位开始截取getQuery('map')
        const mapName = getQuery('map').substring(4);
        currWarMap = mapName === 'fby' ? 'hdz': mapName;
        console.log('currWarMap', currWarMap);
        changeWarMap(currWarMap, 'pc');
        
    } else {
        changeWarMap('pc', 'pc');
    }
    
   


    // initNav();
    // bindOptionEvent();
    $('.war-lv-change-ctn').addClass('show')
    $('.select-region-ctn').css('display', 'none')
 }

// 视角切换
function viewChangeToMap () {

    $.each(cacheMarker, function (index) {
        if (this.options.icon.name === "进攻方基地" ) {
            // console.log(`${window.viewChange ? 'g_jdbsd_g': 'g_jdbsd_r'}`);
            var icon = L.divIcon({
                className: ` map-war-icon`,
                html: `<div class="map-icon-bg"><img src="//game.gtimg.cn/images/dfm/cp/a20240729directory/img/dzc_i/${window.viewChange ? 'g_jdbsd_g': 'g_jdbsd_r'}.png"/></div>`,
                iconSize: [30, 30],			//设置图标大小
                iconAnchor: [15, 15],		//设置图标偏移
            })
            icon.name = '进攻方基地'
            this.setIcon(icon);
        } else if (this.options.icon.name === "防守方基地") {
            var icon = L.divIcon({
                className: ` map-war-icon`,
                html: `<div class="map-icon-bg"><img src="//game.gtimg.cn/images/dfm/cp/a20240729directory/img/dzc_i/${window.viewChange ? 'f_jdbsd_r': 'f_jdbsd_g'}.png"/></div>`,
                iconSize: [30, 30],			//设置图标大小
                iconAnchor: [15, 15],		//设置图标偏移
            })
            icon.name = '防守方基地'
            this.setIcon(icon);
        }
    })
}

// 查找是否有楼层
function findFloor (regionName) {
    let floorInfo = mapScaleInfo.floor.find(item => item.floor_name === regionName);
    if (floorInfo) {
        return true;
    }
}

// 重制楼层
function resetFloor () {
    $('.map-floor-change-ctn').removeClass('show')
    isFloor = false;
    $('.map-floor .btn-floor').html(`<p>查看地图分层</p>`)
    $('.map-floor .btn-floor').attr('class', 'btn-floor btn_floor')
    currFloorIndex = -1;
    $('#MapContainer').removeClass('floor')
}

// 地图难度切换
function changeMapLv(type) {
  initDitu(type)
    console.log('changeMapLv', type);
    
    // 地图配置映射表
    const mapConfigs = {
        // 零号大坝配置
        '00': {
            info: dabaInfo,
            nav: navList,
            navInfo: navListInfo,
            icons: mapArticle,
            poi: selectRegion,
            name: '零号大坝',
            level: '常规',
            layer: 'map_db',
            needRemove: true,
        },
        '00_B1': {
            info: dabaInfo,
            nav: dabaInfo.floorInfo.navList_minus,
            navInfo: dabaInfo.floorInfo.navList_minus,
            icons: dabaInfo.floorInfo.mapArticle_minus,
            poi: selectRegion,
            name: '零号大坝',
            level: '常规',
            layer: 'daba_0f',
            needRemove: true
        },
        '00_1F': {
            info: dabaInfo,
            nav: dabaInfo.floorInfo.navList_firest,
            navInfo: dabaInfo.floorInfo.navList_firest,
            icons: dabaInfo.floorInfo.mapArticle_first,
            poi: selectRegion,
            name: '零号大坝',
            level: '常规',
            layer: 'daba_1f',
            needRemove: true
        },
        '00_2F': {
            info: dabaInfo,
            nav: dabaInfo.floorInfo.navList_second,
            navInfo: dabaInfo.floorInfo.navList_second,
            icons: dabaInfo.floorInfo.mapArticle_second,
            poi: selectRegion,
            name: '零号大坝',
            level: '常规',
            layer: 'daba_2f',
            needRemove: true
        },
        '01': {
            info: dabaInfo,
            nav: navList2,
            navInfo: navListInfo2,
            icons: mapArticle2,
            poi: selectRegion,
            name: '零号大坝',
            level: '机密',
            layer: 'map_db',
            needRemove: true
        },
        '01_B1': {
            info: dabaInfo,
            nav: dabaInfo.floorInfo.navList2_minus,
            navInfo: dabaInfo.floorInfo.navList2_minus,
            icons: dabaInfo.floorInfo.mapArticle2_minus,
            poi: selectRegion,
            name: '零号大坝',
            level: '机密',
            layer: 'daba_0f',
            needRemove: true
        },
        '01_1F': {
            info: dabaInfo,
            nav: dabaInfo.floorInfo.navList2_firest,
            navInfo: dabaInfo.floorInfo.navList2_firest,
            icons: dabaInfo.floorInfo.mapArticle2_first,
            poi: selectRegion,
            name: '零号大坝',
            level: '机密',
            layer: 'daba_1f',
            needRemove: true
        },
        '01_2F': {
            info: dabaInfo,
            nav: dabaInfo.floorInfo.navList2_second,
            navInfo: dabaInfo.floorInfo.navList2_second,
            icons: dabaInfo.floorInfo.mapArticle2_second,
            poi: selectRegion,
            name: '零号大坝',
            level: '机密',
            layer: 'daba_2f',
            needRemove: true
        },
        '02': {
            info: dabaInfo,
            nav: navList3,
            navInfo: navListInfo3,
            icons: mapArticle3,
            poi: selectRegion,
            name: '零号大坝',
            level: '终夜',
            layer: 'map_db',
            needRemove: true
        },
        '02_B1': {
            info: dabaInfo,
            nav: dabaInfo.floorInfo.navList3_minus,
            navInfo: dabaInfo.floorInfo.navList3_minus,
            icons: dabaInfo.floorInfo.mapArticle3_minus,
            poi: selectRegion,
            name: '零号大坝',
            level: '终夜',
            layer: 'daba_0f',
            needRemove: true
        },
        '02_1F': {
            info: dabaInfo,
            nav: dabaInfo.floorInfo.navList3_firest,
            navInfo: dabaInfo.floorInfo.navList3_firest,
            icons: dabaInfo.floorInfo.mapArticle3_first,
            poi: selectRegion,
            name: '零号大坝',
            level: '终夜',
            layer: 'daba_1f',
            needRemove: true
        },
        '02_2F': {
            info: dabaInfo,
            nav: dabaInfo.floorInfo.navList3_second,
            navInfo: dabaInfo.floorInfo.navList3_second,
            icons: dabaInfo.floorInfo.mapArticle3_second,
            poi: selectRegion,
            name: '零号大坝',
            level: '终夜',
            layer: 'daba_2f',
            needRemove: true
        },
        // 长弓溪谷配置
        '10': {
            info: cgxgInfo,
            nav: navList_cgxg,
            navInfo: navListInfo_cgxg,
            icons: mapArticle_cgxg,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '常规',
            layer: 'map_yc',
            needRemove: true
        },
        '10_1F': {
            info: cgxgInfo,
            nav: cgxgInfo.floorInfo.navList_firest,
            navInfo: cgxgInfo.floorInfo.navList_firest,
            icons: cgxgInfo.floorInfo.mapArticle_first,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '常规',
            layer: 'cgxg_1f',
            needRemove: true
        },
        '10_2F': {
            info: cgxgInfo,
            nav: cgxgInfo.floorInfo.navList_second,
            navInfo: cgxgInfo.floorInfo.navList_second,
            icons: cgxgInfo.floorInfo.mapArticle_second,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '常规',
            layer: 'cgxg_2f',
            needRemove: true
        },
        '10_s': {
            info: cgxgInfo,
            nav: navList2_cgxg,
            navInfo: navListInfo2_cgxg,
            icons: mapArticle2_cgxg,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '常规｜坠机事件',
            layer: 'map_yc2',
            needRemove: true
        },
        '10_s_1F': {
            info: cgxgInfo,
            nav: cgxgInfo.floorInfo.navList_s_firest,
            navInfo: cgxgInfo.floorInfo.navList_s_firest,
            icons: cgxgInfo.floorInfo.mapArticle_s_first,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '常规｜坠机事件',
            layer: 'cgxg_1f',
            needRemove: true
        },
        '10_s_2F': {
            info: cgxgInfo,
            nav: cgxgInfo.floorInfo.navList_s_second,
            navInfo: cgxgInfo.floorInfo.navList_s_second,
            icons: cgxgInfo.floorInfo.mapArticle_s_second,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '常规｜坠机事件',
            layer: 'cgxg_2f',
            needRemove: true
        },
        '11': {
            info: cgxgInfo,
            nav: navList3_cgxg,
            navInfo: navListInfo3_cgxg,
            icons: mapArticle3_cgxg,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '机密',
            layer: 'map_yc',
            needRemove: true
        },
        '11_1F': {
            info: cgxgInfo,
            nav: cgxgInfo.floorInfo.navList2_firest,
            navInfo: cgxgInfo.floorInfo.navList2_firest,
            icons: cgxgInfo.floorInfo.mapArticle2_first,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '机密',
            layer: 'cgxg_1f',
            needRemove: true
        },
        '11_2F': {
            info: cgxgInfo,
            nav: cgxgInfo.floorInfo.navList2_second,
            navInfo: cgxgInfo.floorInfo.navList2_second,
            icons: cgxgInfo.floorInfo.mapArticle2_second,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '机密',
            layer: 'cgxg_2f',
            needRemove: true
        },
        '11_s': {
            info: cgxgInfo,
            nav: navList4_cgxg,
            navInfo: navListInfo4_cgxg,
            icons: mapArticle4_cgxg,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '机密｜坠机事件',
            layer: 'map_yc2',
            needRemove: true
        },
        '11_s_1F': {
            info: cgxgInfo,
            nav: cgxgInfo.floorInfo.navList2_s_firest,
            navInfo: cgxgInfo.floorInfo.navList2_s_firest,
            icons: cgxgInfo.floorInfo.mapArticle2_s_first,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '机密｜坠机事件',
            layer: 'cgxg_1f',
            needRemove: true
        },
        '11_s_2F': {
            info: cgxgInfo,
            nav: cgxgInfo.floorInfo.navList2_s_second,
            navInfo: cgxgInfo.floorInfo.navList2_s_second,
            icons: cgxgInfo.floorInfo.mapArticle2_s_second,
            poi: selectRegion_cgxg,
            name: '长弓溪谷',
            level: '机密｜坠机事件',
            layer: 'cgxg_2f',
            needRemove: true
        },
        // 航天基地配置
        '21': {
            info: htjdInfo,
            nav: navList_htjd,
            navInfo: navListInfo_htjd,
            icons: mapArticle_htjd,
            poi: selectRegion_htjd,
            name: '航天基地',
            level: '机密',
            layer: 'map_htjd',
            needRemove: true
        },
        '21_s': {
            info: htjdInfo,
            nav: navList2_htjd,
            navInfo: navListInfo2_htjd,
            icons: mapArticle2_htjd,
            poi: selectRegion_htjd,
            name: '航天基地',
            level: '机密|断桥事件',
            layer: 'map_htjd2',
            needRemove: true
        },
        '22': {
            info: htjdInfo,
            nav: navList3_htjd,
            navInfo: navListInfo3_htjd,
            icons: mapArticle3_htjd,
            poi: selectRegion_htjd,
            name: '航天基地',
            level: '绝密',
            layer: 'map_htjd',
            needRemove: true
        },
        '22_s': {
            info: htjdInfo,
            nav: navList4_htjd,
            navInfo: navListInfo4_htjd,
            icons: mapArticle4_htjd,
            poi: selectRegion_htjd,
            name: '航天基地',
            level: '绝密|断桥事件',
            layer: 'map_htjd2',
            needRemove: true
        },
        
        // 巴克什配置
        '30': {
            info: bksInfo,
            nav: navList_bks,
            navInfo: navListInfo_bks,
            icons: mapArticle_bks,
            poi: selectRegion_bks,
            name: '巴克什',
            level: '常规',
            layer: 'map_bks2',
            needRemove: true
        },
        '31': {
            info: bksInfo,
            nav: navList_bks,
            navInfo: navListInfo_bks,
            icons: mapArticle_bks,
            poi: selectRegion_bks,
            name: '巴克什',
            level: '机密',
            layer: 'map_bks2',
            needRemove: true
        },
        '31_B1': {
            info: bksInfo,
            nav: bksInfo.floorInfo.navList_firest,
            navInfo: bksInfo.floorInfo.navList_firest,
            icons: bksInfo.floorInfo.mapArticle_first,
            poi: selectRegion_bks,
            name: '巴克什',
            level: '机密',
            // layer: 'map_bks2',
            layer: 'bks_1f',
            needRemove: true
        },
        '31_1F': {
            info: bksInfo,
            nav: bksInfo.floorInfo.navList_second,
            navInfo: bksInfo.floorInfo.navList_second,
            icons: bksInfo.floorInfo.mapArticle_second,
            poi: selectRegion_bks,
            name: '巴克什',
            level: '机密',
            // layer: 'map_bks2',
            layer: 'bks_2f',
            needRemove: true
        },
        '31_2F': {
            info: bksInfo,
            nav: bksInfo.floorInfo.navList_three,
            navInfo: bksInfo.floorInfo.navList_three,
            icons: bksInfo.floorInfo.mapArticle_three,
            poi: selectRegion_bks,
            name: '巴克什',
            level: '机密',
            // layer: 'map_bks2',
            layer: 'bks_3f',
            needRemove: true
        },
        '32': {
            info: bksInfo,
            nav: navList2_bks,
            navInfo: navListInfo2_bks,
            icons: mapArticle2_bks,
            poi: selectRegion_bks,
            name: '巴克什',
            level: '绝密',
            layer: 'map_bks2',
            needRemove: true
        },
        '32_B1': {
            info: bksInfo,
            nav: bksInfo.floorInfo.navList2_firest,
            navInfo: bksInfo.floorInfo.navList2_firest,
            icons: bksInfo.floorInfo.mapArticle2_first,
            poi: selectRegion_bks,
            name: '巴克什',
            level: '绝密',
            // layer: 'map_bks2',
            layer: 'bks_1f',
            needRemove: true
        },
        '32_1F': {
            info: bksInfo,
            nav: bksInfo.floorInfo.navList2_second,
            navInfo: bksInfo.floorInfo.navList2_second,
            icons: bksInfo.floorInfo.mapArticle2_second,
            poi: selectRegion_bks,
            name: '巴克什',
            level: '绝密',
            // layer: 'map_bks2',
            layer: 'bks_2f',
            needRemove: true
        },
        '32_2F': {
            info: bksInfo,
            nav: bksInfo.floorInfo.navList2_three,
            navInfo: bksInfo.floorInfo.navList2_three,
            icons: bksInfo.floorInfo.mapArticle2_three,
            poi: selectRegion_bks,
            name: '巴克什',
            level: '绝密',
            // layer: 'map_bks2',
            layer: 'bks_3f',
            needRemove: true
        },
        // 潮汐监狱配置
        '42': {
            info: cxjyInfo,
            nav: navList_cxjy,
            navInfo: navListInfo_cxjy,
            icons: mapArticle_cxjy,
            poi: selectRegion_cxjy,
            name: '潮汐监狱',
            level: '绝密',
            layer: 'map_cxjy',
            needRemove: true
        },
        '42_1F': {
            info: cxjyInfo,
            nav: cxjyInfo.floorInfo.navList_firest,
            navInfo: cxjyInfo.floorInfo.navList_firest,
            icons: cxjyInfo.floorInfo.mapArticle_first,
            poi: selectRegion_cxjy,
            name: '潮汐监狱',
            level: '绝密',
            layer: 'cxjy_1f',
            needRemove: true
        },
        '42_2F': {
            info: cxjyInfo,
            nav: cxjyInfo.floorInfo.navList_second,
            navInfo: cxjyInfo.floorInfo.navList_second,
            icons: cxjyInfo.floorInfo.mapArticle_second,
            poi: selectRegion_cxjy,
            name: '潮汐监狱',
            level: '绝密',
            layer: 'cxjy_2f',
            needRemove: true
        },
        '42_3F': {
            info: cxjyInfo,
            nav: cxjyInfo.floorInfo.navList_three,
            navInfo: cxjyInfo.floorInfo.navList_three,
            icons: cxjyInfo.floorInfo.mapArticle_three,
            poi: selectRegion_cxjy,
            name: '潮汐监狱',
            level: '绝密',
            layer: 'cxjy_3f',
            needRemove: true
        },
        '42_4F': {
            info: cxjyInfo,
            nav: cxjyInfo.floorInfo.navList_four,
            navInfo: cxjyInfo.floorInfo.navList_four,
            icons: cxjyInfo.floorInfo.mapArticle_four,
            poi: selectRegion_cxjy,
            name: '潮汐监狱',
            level: '绝密',
            layer: 'cxjy_4f',
            needRemove: true
        },
    };

    // 获取当前地图配置
    const config = mapConfigs[type];
    // console.log("mapConfigs", type);
    // if (getQuery('map') === 'daba') {
    //      currMap = '0'
    // }
    // if (getQuery('map') === 'cgxg') {
    //     currMap = '1'
    // }
    // if (getQuery('map') === 'bks') {
    //     currMap = '3'
    // }
    if (!config) {
        toastTips();
        isZj = false;
        return;
    }
    
    // 更新地图状态
    mapScaleInfo = config.info;
    allNavList = config.nav;
    navTypeList = config.navInfo;
    mapIcons = config.icons;
    poiInfo = config.poi;
    
    // 更新按钮视觉状态
    isZj ? $('.btn-random').addClass('open') : $('.btn-random').removeClass('open');
    if (!isFloor) {
        $('.map-floor-change-ctn').removeClass('show');
    }
    // 更新UI

    if (chooseItemLvName.indexOf('夜') > -1) {
        $('.curr-map-name').html(`${config.name} <span class="map-lv"> ( ${chooseItemLvName} )</span>`);
    } else {
        $('.curr-map-name').html(`${config.name} <span class="map-lv"> ( ${config.level} )</span>`);
    }
  
    
    // 切换地图图层
    if (config.needRemove) {
        map.removeLayer(currLayer);
    }
    if (currLayer.name !== config.layer) {
        addLayer(config.layer);
    }
    if (config.needRemove) {
        map.addLayer(currLayer);
    }

    // 重置标记状态
    typeListInit = false;
    if(outFloor) {
        resetAll('none');
    }

    // 更新按钮状态和可见性
    if (isAll) {
        
        toggleVisible(`${currLeftNav}_all`, currLeftNav);
        $('.btn-choose-all-icon').attr('class', 'img_all_open btn-choose-all-icon');
    } else {
        // if (!isFloor && outFloor) {
        //     toggleVisible(`${currLeftNav}_none`, currLeftNav);
        //     $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon');
        // }

        // if (!isAll) {
        //     toggleVisible(`${currLeftNav}_none`, currLeftNav);
        //     $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon');
        // }
        if(outFloor) {
            toggleVisible(`none`, currLeftNav);
            $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon');
        }

       

        
    }

    // 渲染导航类型列表
    if (!isFloor) {
        renderNavTypeList(
            Number(currLeftNav) === 0 ? allNavList[0].typeList : navTypeList[currLeftNav].typeList,
            currLeftNav
        );
    } else {
        renderNavTypeList(allNavList, 0);
    }

    bindOptionEvent();
}


// 战场切换
function changeWarMap(mapName, type) {
    visibleMarker = {}
    mapScaleInfo = window[mapName].info;
    poiInfo =  window[mapName].region;
    if (window.occupy) {
        mapIcons = window[`${mapName}_${type}_s`].mapArticle;
        allNavList = window[`${mapName}_${type}_s`].navRegion;
        navTypeList = window[`${mapName}_${type}_s`].navRegionInfo;
        if (currLayer.name !== mapScaleInfo.names) {
            map.removeLayer(currLayer)
            addLayer(mapScaleInfo[`names_${currWarType}`])
        }
    } else {
        mapIcons = window[`${mapName}_${type}`].mapArticle[window.warLv];
        allNavList = window[`${mapName}_${type}`].navRegion[window.warLv];
        navTypeList = window[`${mapName}_${type}`].navRegionInfo[window.warLv];
        if (currLayer.name !== mapScaleInfo.name) {
            map.removeLayer(currLayer)
            addLayer(mapScaleInfo[`name_${currWarType}`])
        }
    }

    const title = !window.occupy ? window[`${mapName}_${type}`].title : window[`${mapName}_${type}_s`].title
    $('.curr-map-name').html(title)



    typeListInit = false;

    initNav();
    warInit(currWarMap, currWarType);


    bindOptionEvent();
}

// 战场初始化
function warInit (mapName, type, isBorder = false) {

    var init;
    init = window.occupy ? window[`${mapName}_${type}_s`].init : window[`${mapName}_${type}`].init
    // if (window.viewChange) {
    //     init = window.occupy ? window[`${mapName}_${type}_s`].init_g : window[`${mapName}_${type}`].init_g
    // } else {
    //     init = window.occupy ? window[`${mapName}_${type}_s`].init_s : window[`${mapName}_${type}`].init_s
    // }
    if (!window.isViewChange) {
        listIsAll[currLeftNav] = false;
        $('.btn-choose-all-icon').attr('class', 'img_all_close btn-choose-all-icon')
    }


    $.each(borderList, function () {
        this.remove();

    });

    if (!isBorder) {
        $.each(warMark, function () {
            this.remove();

        });

        $.each(cacheMarker, function () {
            this.remove();

        });
    } else {
        $.each(cacheMarker, function () {
            // console.log(111, this.options);
            if (this.options.icon.name) {
                if (this.options.icon.name.indexOf('据点') > -1) {

                    this.remove();
                }
            }
            if (this.options.icon.options.html.indexOf('jd') > -1) {
                this.remove();
            }


        });
    }



    // console.log('cacheMarker', cacheMarker);


    // visibleMarker = []
    // cacheMarker = []

    borderList = []

    var initList = [];
    // 是否是占领模式
    if (window.occupy) {
        initList = init
    } else {
        initList = init[window.warLv].typeList
    }

    let lineHtmlg = ''
    let lineHtmlf = ''
    let lineHtmlz = ''
    let lineHtml = ''

    // console.log(window[`${currWarMap}`]);

    for (let index = 0; index < window[`${currWarMap}`].info.sector; index++) {
        lineHtml += `
            <div class="war-lv-item war-lv-item-${index} ${window.warLv === index && 'active'}" data-index="${index}"></div>
        `

    }
    $('.war-lv-change-list').html(lineHtml)

    $('.war-lv-item').on('click', (e) => {
        var index = $(e.target).attr('data-index')
        window.warLv = Number(index);
        changeWarMap(currWarMap, currWarType);
        // initNav();
        // bindOptionEvent();
        $('.lv-change-tips').text(`区域${warLvText[window.warLv]}`)
        $('.deploy-swiper').removeClass('show')
        map.flyTo([window[currWarMap].info.sectorInit[window.warLv].initX,  window[currWarMap].info.sectorInit[window.warLv].initY], window[currWarMap].info.sectorInit[window.warLv].initZoom)
    })

    // // console.log('init.border', window[mapName].info);
    // testWarInit(init);
    // return;
    // drawBorderPub('red', window[mapName].info.border, window[mapName].info.border2)
    for (let index = 0; index < initList.length; index++) {
        const element = initList[index];

        if (element.border) {
            // if (element.region.indexOf('进攻') > -1) {
            //     drawBorder('red', element.border)
            // }

            //  if (element.region.indexOf('防守') > -1){
            //     drawBorder('green', element.border)
            // }

            // if (element.isRegion === 'true') {
            //     drawBorder('white', element.border)
            // }
            if (element.name.indexOf('据点') > -1) {
                drawBorder(window.occupy ? 'white' : 'green', element.border, true)
            } else {
                if (element.region.indexOf('进攻') > -1 || element.region.indexOf('GTI') > -1) {
                    drawBorder(window.viewChange ? 'green' : 'red', element.border)
                } else if (element.region.indexOf('防守') > -1 || element.region.indexOf('HAAVK') > -1){
                    drawBorder(window.viewChange ? 'red' : 'green', element.border)
                } else {
                    drawBorder('white', element.border)
                }
            }



        }

        if (element.isRegion === "false") {
            // console.log(element.x, element.y);

            var pos = getMapPos(element.x, element.y)
            let myIcon;
            if (element.name.indexOf('据点') > -1) {
                lineHtmlz += `<div class="war-lv-jd img_nav_jd_${element['自定义区域']}"></div>`
            } else {
                if (element.region.indexOf('进攻') > -1 || element.region.indexOf('GTI') > -1) {
                    lineHtmlg += `<div class="war-lv-item ${window.viewChange ? 'green' : 'red'}"></div>`
                } else if (element.region.indexOf('防守') > -1 || element.region.indexOf('HAAVK') > -1){
                    lineHtmlf += `<div class="war-lv-item ${window.viewChange ? 'red' : 'green'}"></div>`
                }
            }
           let icon;
           if (element.name === "进攻方基地" ) {
                icon = window.viewChange ? 'g_jdbsd_g': 'g_jdbsd_r'
            } else if (element.name === "防守方基地") {
                icon = window.viewChange ? 'f_jdbsd_r': 'f_jdbsd_g'
            } else {
                icon = element.icon
            }

            if (element.rotate) {
                let rotate = currWarMap === 'qhz' ? 90 : 180
                // console.log('rotate', currWarMap);

                myIcon = L.divIcon({
                    className: ` map-war-icon`,
                    html: `<div class="map-icon-bg" style="transform: translate3d(-50%, -50%, 0)"><img src="//game.gtimg.cn/images/dfm/cp/a20240729directory/img/dzc_i/${icon}.png" style="transform:  rotate(${Number(element.rotate) + rotate}deg)"/></div>`,
                    iconSize: [30, 30],			//设置图标大小
                    iconAnchor: [15, 15],		//设置图标偏移
                })
            } else {
                myIcon = L.divIcon({
                    className: ` map-war-icon`,
                    html: `<div class="map-icon-bg"><img src="//game.gtimg.cn/images/dfm/cp/a20240729directory/img/dzc_i/${icon}.png"/></div>`,
                    iconSize: [30, 30],			//设置图标大小
                    iconAnchor: [15, 15],		//设置图标偏移
                })
            }
            myIcon.name = element.name;
            myIcon.icon = icon;
            visibleMarker[element.name] = true;
            $(`.nav-list-nav${icon.substring(1)}`).addClass('active')
            if (element['激活条件']) {
                var popupHtml =`
                    <div class="name">${element.name}</div>
                     <div class="open-text war ${(!element['激活条件'] || element['激活条件'] === '' || element['激活条件'] === '-') ? 'hide': ''}">激活条件：<span>${element['激活条件']}</span></div>
                     `
            } else {
                var popupHtml = `
                <div class="name">${element.name}</div>
            `;
            }
           
            // console.log('pos', pos);
            
            // popupHtml += '</div>';
            cacheMarker.push(L.marker([pos.y, pos.x], {icon: myIcon}).bindPopup(popupHtml).addTo(map).on({
                click: function () {
                    currClickMarker?.setIcon(currClickMarker?.myIcon)
                    this.isClick = true;
                    this.myIcon = myIcon;
                    // console.log(this);
                    let rotate = currWarMap === 'qhz' ? 90 : 180
                    this.openPopup();
                    this.setIcon( L.divIcon({
                        className: ` map-war-icon click`,
                        html: `<div class="map-icon-bg" style="${element?.rotate ? `transform: translate3d(-50%, -50%, 0) rotate(${Number(element?.rotate) + rotate}deg)` : ''}"><img src="//game.gtimg.cn/images/dfm/cp/a20240729directory/img/dzc_i/${icon}.png"/></div>`,
                        iconSize: [30, 30],			//设置图标大小
                        iconAnchor: [15, 15],		//设置图标偏移
                    }));
                    currClickMarker = this;
                    $(this.getElement()).addClass('click')
                    
                    console.log(this.myIcon);
                    
                    if (this.myIcon.name.indexOf('基地') > -1 || (this.myIcon.name.indexOf('据点') > -1 && window.occupy)) {
                        console.log('initWarSwiper', this.myIcon.name, element);
                        initWarSwiper(this.myIcon.name, element);
                    }
                    
                    // this?.remove()
                }
            }))
            
            
           
        }

       
    }

    
}

function testWarInit (list) {
    let lineHtmlg = ''
    let lineHtmlf = ''
    let lineHtmlz = ''
    let lineHtml = ''
    for (let index_f = 0; index_f < list.length; index_f++) {
        const element_f = list[index_f];
        // console.log(111, element_f);
        
        for (let index = 0; index < element_f.typeList.length; index++) {
            const element = element_f.typeList[index];
            // console.log(222, element);
            
            if (element.border) {
                // if (element.region.indexOf('进攻') > -1) {
                //     drawBorder('red', element.border)
                // }
    
                //  if (element.region.indexOf('防守') > -1){
                //     drawBorder('green', element.border)
                // }
    
                // if (element.isRegion === 'true') {
                //     drawBorder('white', element.border)
                // }
    
                if (element.name.indexOf('据点') > -1) {
                    drawBorder(window.occupy ? 'white' : 'green', element.border, true)
                } else {
                    if (element.region.indexOf('进攻') > -1 || element.region.indexOf('GTI') > -1) {
                        drawBorder(window.viewChange ? 'green' : 'red', element.border)
                    } else if (element.region.indexOf('防守') > -1 || element.region.indexOf('HAAVK') > -1){
                        drawBorder(window.viewChange ? 'red' : 'green', element.border)
                    } else {
                        drawBorder('white', element.border)
                    }
                }
    
    
    
            }
    
            if (element.isRegion === "false") {
                // console.log(element.x, element.y);
    
                var pos = getMapPos(element.x, element.y)
                let myIcon;
                if (element.name.indexOf('据点') > -1) {
                    lineHtmlz += `<div class="war-lv-jd img_nav_jd_${element['自定义区域']}"></div>`
                } else {
                    if (element.region.indexOf('进攻') > -1 || element.region.indexOf('GTI') > -1) {
                        lineHtmlg += `<div class="war-lv-item ${window.viewChange ? 'green' : 'red'}"></div>`
                    } else if (element.region.indexOf('防守') > -1 || element.region.indexOf('HAAVK') > -1){
                        lineHtmlf += `<div class="war-lv-item ${window.viewChange ? 'red' : 'green'}"></div>`
                    }
                }
               let icon;
               if (element.name === "进攻方基地" ) {
                    icon = window.viewChange ? 'g_jdbsd_g': 'g_jdbsd_r'
                } else if (element.name === "防守方基地") {
                    icon = window.viewChange ? 'f_jdbsd_r': 'f_jdbsd_g'
                } else {
                    icon = element.icon
                }
    
                if (element.rotate) {
                    let rotate = currWarMap === 'qhz' ? 90 : 180
                    // console.log('rotate', currWarMap);
    
                    myIcon = L.divIcon({
                        className: ` map-war-icon`,
                        html: `<div class="map-icon-bg" style="transform: translate3d(-50%, -50%, 0)"><img src="../../img/dzc_i/${icon}.png" style="transform:  rotate(${Number(element.rotate) + rotate}deg)"/></div>`,
                        iconSize: [30, 30],			//设置图标大小
                        iconAnchor: [15, 15],		//设置图标偏移
                    })
                } else {
                    myIcon = L.divIcon({
                        className: ` map-war-icon`,
                        html: `<div class="map-icon-bg"><img src="../../img/dzc_i/${icon}.png"/></div>`,
                        iconSize: [30, 30],			//设置图标大小
                        iconAnchor: [15, 15],		//设置图标偏移
                    })
                }
                myIcon.name = element.name;
                myIcon.icon = icon;
                visibleMarker[element.name] = true;
                $(`.nav-list-nav${icon.substring(1)}`).addClass('active')
                if (element['激活条件']) {
                    var popupHtml =`
                        <div class="name">${element.name}</div>
                         <div class="open-text war ${(!element['激活条件'] || element['激活条件'] === '' || element['激活条件'] === '-') ? 'hide': ''}">激活条件：<span>${element['激活条件']}</span></div>
                         `
                } else {
                    var popupHtml = `
                    <div class="name">${element.name}</div>
                `;
                }
               
                // console.log('pos', pos);
                
                // popupHtml += '</div>';
                cacheMarker.push(L.marker([pos.y, pos.x], {icon: myIcon}).bindPopup(popupHtml).addTo(map).on({
                    click: function () {
                        currClickMarker?.setIcon(currClickMarker?.myIcon)
                        this.isClick = true;
                        this.myIcon = myIcon;
                        // console.log(this);
                        let rotate = currWarMap === 'qhz' ? 90 : 180
                        this.openPopup();
                        this.setIcon( L.divIcon({
                            className: `${isWar ? 'map-war-icon' : 'map-icon'} click`,
                            html: `<div class="map-icon-bg" style="${element?.rotate ? `transform: translate3d(-50%, -50%, 0) rotate(${Number(element?.rotate) + rotate}deg)` : ''}"><img src="//game.gtimg.cn/images/dfm/cp/a20240729directory/img/dzc_i/${icon}.png"/></div>`,
                            iconSize: [30, 30],			//设置图标大小
                            iconAnchor: [15, 15],		//设置图标偏移
                        }));
                        currClickMarker = this;
                        $(this.getElement()).addClass('click')
                        
                        // console.log(this.myIcon);
                        
                        if (this.myIcon.name.indexOf('基地') > -1 || (this.myIcon.name.indexOf('据点') > -1 && window.occupy)) {
                            initWarSwiper(this.myIcon.name, element);
                        }
                        
                        // this?.remove()
                    }
                }))
                
                
               
            }
    
           
        }
        
    }

}

// 清除战场
function warRemove () {
    $.each(borderList, function () {
        this.remove();
    
    });
    $.each(warMark, function () {
        this.remove();
    
    });
}

// 部署swiper
function initWarSwiper (name, data) {
    // 如果存在，则销毁
    if (window.warSwiper) {
        try {
            // 兼容不同版本的Swiper
            if (typeof window.warSwiper.destroy === 'function') {
                window.warSwiper.destroy(true, true); // 对于新版本可能需要多个参数
            } else if (typeof window.warSwiper.destroy$ === 'function') {
                window.warSwiper.destroy$(); // 某些版本可能使用这个方法
            } else {
                // 如果没有destroy方法，尝试手动清理
                window.warSwiper = null;
            }
        } catch (error) {
            // 出错时确保swiper被重置
            // console.error('销毁Swiper出错:', error);
            window.warSwiper = null;
        }
    }


    let html = '';
    // console.log(data);
    let list = window.occupy ? window[currWarMap + '_' + currWarType + '_s'].deploy : window[currWarMap + '_' + currWarType].deploy[window.warLv]
    console.log(name, list);
    // if (!list) return;
    let length = 0;
    
   
    $.each(list, function(index) {
        console.log('this', name.indexOf(this['阵营']), this['阵营'], name, this['阵营'] === data['name']);
        
        if (name.indexOf(this['阵营']) > -1 && !window.occupy) {
            length++;
            if (this?.type) {
                html += ` <div class="swiper-slide">
                <div class="slide-name">${this.name}</div>
                <div class="slide-bot">
                    <div class="slide-img-ctn">
                        <div class="slide-img ${this.icon}"></div>
                    </div>
                    <div class="slide-info">
                        <div class="slide-help">所需积分:${this.num}</div>
                    </div>
                </div>
            </div>`
            } else if (this['阵营'] === data['name']) { 
                html += ` <div class="swiper-slide">
                <div class="slide-name">${this.name}</div>
                <div class="slide-bot">
                    <div class="slide-img-ctn">
                        <div class="slide-img ${this.icon}"></div>
                    </div>
                    <div class="slide-info">
                        <div class="slide-tiem">${this.CD}s</div>
                        <div class="slide-num">可部署:${this.num}</div>
                    </div>
                </div>
            </div>`
            } else {
                if (this['备注'] !== data['自定义区域']) return;
                html += ` <div class="swiper-slide">
                <div class="slide-name">${this.name}</div>
                <div class="slide-bot">
                    <div class="slide-img-ctn">
                        <div class="slide-img ${this.icon}"></div>
                    </div>
                    <div class="slide-info">
                        <div class="slide-tiem">${this.CD}s</div>
                        <div class="slide-num">可部署:${this.num}</div>
                    </div>
                </div>
            </div>`
            }
        // } else if ((name.indexOf(this['阵营']) > -1) && window.occupy) {
        } else if (this['备注'] === data['自定义区域'] && window.occupy) {
            // console.log('有', this['备注'], data['自定义区域']);
            
            length++;
                html += ` <div class="swiper-slide">
                <div class="slide-name">${this.name}</div>
                <div class="slide-bot">
                    <div class="slide-img-ctn">
                        <div class="slide-img ${this.icon}"></div>
                    </div>
                    <div class="slide-info">
                        <div class="slide-tiem">${this.CD}s</div>
                        <div class="slide-num">可部署:${this.num}</div>
                    </div>
                </div>
            </div>`
        } else if (this['阵营'] === data['name']) {
            html += ` <div class="swiper-slide">
                <div class="slide-name">${this.name}</div>
                <div class="slide-bot">
                    <div class="slide-img-ctn">
                        <div class="slide-img ${this.icon}"></div>
                    </div>
                    <div class="slide-info">
                        <div class="slide-tiem">${this.CD}s</div>
                        <div class="slide-num">可部署:${this.num}</div>
                    </div>
                </div>
            </div>`
        }
        // // console.log(this['备注'], data['自定义区域'], this['备注'] === data['自定义区域']);
        
        

        
    })

    if (length < 7) {
        $('.btn-swiper-prev').css('display', 'none')
        $('.btn-swiper-next').css('display', 'none')
    } else {
        $('.btn-swiper-prev').css('display', 'block')
        $('.btn-swiper-next').css('display', 'block')
    }
    
 
    $('.swiper-wrapper').html(html)

    // 创建Swiper实例
    try {
        window.warSwiper = new Swiper('.swiper-container', {
            slidesPerView: 'auto',
            // 添加其他必要的选项，确保兼容性
            observer: true, // 启用DOM变化监听
            observeParents: true,
        });

        // console.log('initSwiper');
    } catch (error) {
        // console.error('创建Swiper失败:', error);
        // 确保即使创建失败也能显示内容
        if (html !== '') {
            $('.deploy-swiper').addClass('show');
        }
        return;
    }
    
    if (html !== '') {
        $('.deploy-swiper').addClass('show')
    } else {
        $('.deploy-swiper').removeClass('show')
    }
}
window.addEventListener('load', () => {
    init();
    // console.log(document.querySelector('.left-nav-ctn'));
});

/**
 * 节流函数 - 限制函数在指定时间内只执行一次
 * @param {Function} fn - 要执行的函数
 * @param {number} delay - 延迟时间（毫秒）
 * @returns {Function} - 返回节流后的函数
 */
function throttle(fn, delay = 200) {
    let lastCall = 0;
    return function(...args) {
        const now = Date.now();
        if (now - lastCall >= delay) {
            lastCall = now;
            return fn.apply(this, args);
        }
    };
}

/**
 * 判断用户是否滑动到指定点位附近
 * @param {number} targetX - 目标点位的x坐标
 * @param {number} targetY - 目标点位的y坐标
 * @param {number} threshold - 判定为"附近"的距离阈值
 * @returns {boolean} - 如果用户在点位附近则返回true，否则返回false
 */
function isNearPoint(targetX, targetY, threshold = 100) {
    const center = map.getCenter();
    const currentX = center.lat;
    const currentY = center.lng;
    
    const distance = Math.sqrt(
        Math.pow(currentX - targetX, 2) + 
        Math.pow(currentY - targetY, 2)
    );
    
    return distance <= threshold;
}

// 检查是否在任何一个点位附近
function checkNearbyPoints() {
    const center = map.getCenter();
    const currentX = center.lat;
    const currentY = center.lng;
    
    // console.log(currentX, currentY);
    
    let nearbyPoints = [];
    // console.log(pointsOfInterest[currMap]);
    var point = pointsOfInterest[currMap]
    if (!point) return;
    
    // 检查point是否为数组
    if (Array.isArray(point)) {
        // 如果是数组，遍历每个点位进行检测
        point.forEach(singlePoint => {
            var pos = getMapPos(singlePoint.x, singlePoint.y);
            
            const distance = Math.sqrt(
                Math.pow(currentX - pos.y, 2) + 
                Math.pow(currentY - pos.x, 2)
            );
            
            if (distance <= singlePoint.threshold) {
                nearbyPoints.push({...singlePoint, distance});
            }
        });
    } else {
        // 如果是单个对象，按原来的逻辑处理
        var pos = getMapPos(point.x, point.y);
        // console.log(pos.x, pos.y);
        
        const distance = Math.sqrt(
            Math.pow(currentX - pos.y, 2) + 
            Math.pow(currentY - pos.x, 2)
        );
        
        if (distance <= point.threshold) {
            nearbyPoints.push({...point, distance});
        }
    }
    
    // 如果有多个点符合条件，可以返回最近的点
    if (nearbyPoints.length > 0) {
        //console.log('多个点', nearbyPoints);
        
        nearbyPoints.sort((a, b) => a.distance - b.distance);
        return nearbyPoints[0];
    }
    
    return null; // 不在任何点位附近
}


// 创建节流版本的检查函数
// const throttledCheckNearbyPoints = throttle(function() {
//     const nearbyPoint = checkNearbyPoints();
//     checkMoveFloor(nearbyPoint);
// }, 500); // 300毫秒的节流时间，可根据需要调整

function checkMoveFloor (nearbyPoint) {
    //console.log('checkMoveFloor', nearbyPoint);
    
    if (isFloor) return;
    if (nearbyPoint && !isMoveFloor) {
        // console.log(`用户已进入"${nearbyPoint.name}"区域！距离: ${nearbyPoint.distance.toFixed(2)}`);
        // 在这里执行你需要的操作
        isMoveFloor = true;
        initFloor();
        // isFloor = true;
        // $('#MapContainer').addClass('floor')
        currRegion = nearbyPoint.name
        $('.not-event').addClass('act');
        $('.map-floor-change-ctn').addClass('show');
        $('.map-floor .btn-floor').html(`<p>请选择楼层</p>`)
        if (Number(currMap) === 4 && !isFloor) {
            $('.floor-tips').css('display', 'none');
        } else {
            $('.floor-tips').css('display', 'block');
            $('.floor-tips').html(`<p> 您正在查看<span class="span1">${currRegion}</span></p>`)
        }
        $('.curr-reigon').text(`${nearbyPoint.name}`)
        // $('.map-floor').css('display', 'block');
    } else if (!nearbyPoint && isMoveFloor) {
        isMoveFloor = false;
        isFloor = false;
        currFloorIndex = -1;
        $('.curr-reigon').text(`地图快速定位`)
        
        floorList.html('')
        floorList.append(`<div class="floor-item not-event">大地图模式</div>`)
        $('.map-floor .btn-floor').html(`<p>查看地图分层</p>`)
        $('.map-floor-change-ctn').removeClass('show');
        // $('.map-floor').css('display', 'none');
    }
}