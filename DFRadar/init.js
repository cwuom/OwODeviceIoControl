var xW = 357100
var yW = -770800
var dee = 310

function initDitu(type) {
  const ditu = type.slice(0, 1)
  if(ditu === "0") {
    xW = 357100
    yW = -770800
    dee = 310
  }
  if(ditu === "1") {
    xW = 328800
    yW = -640600
    dee = 180
  }
  if(ditu === "2") {
    xW = 668200
    yW = -452923
    dee = 270
  }
  if(ditu === "3") {
    xW = 378400
    yW = -449400
    dee = 280
  }
  if(ditu === "4") {
    xW = 53150
    yW = -52650
    dee = 280
  }

  setTimeout(()=> {
    for(const name of personMap.keys()) {
      const per = personMap.get(name)
      per && map.removeControl(per.marker)
      per && personMap.delete(name)
    }
  }, 1000)
}
