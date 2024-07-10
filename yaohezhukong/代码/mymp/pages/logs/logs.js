// logs.js
const util = require('../../utils/util.js')
var mqtt = require('../../utils/mqtt.min.js');
var client=null
const date = new Date()
const years = []
const months = []
const days = []

for (let i = 0; i <= 3; i++) {
  years.push(i)
}

for (let i = 0; i <= 23; i++) {
  months.push(i)
}

for (let i = 0; i <= 59; i++) {
  days.push(i)
}
Page({
  data: {
    years: years,
    year: 0,
    months: months,
    month: 0,
    days: days,
    day: 0,
    value: [0, 0, 0],
  },
  onLoad(){
    this.connectmqtt()
    // console.log('1')
  },
  bindChange: function (e) {
    const val = e.detail.value
    this.setData({
      year: this.data.years[val[0]],
      month: this.data.months[val[1]],
      day: this.data.days[val[2]]
    })
  },
  setClock:function(){
    var that=this
    client.publish('/iot/5898/sub/1', '{ "service_id": "Health_System", "command_name": "Medicine_Clock", "paras": { "Clock": "'+that.data.year+'/'+that.data.month+'/'+that.data.day+'/00" } }',  function(err){
      if (!err) {
        console.log('发送成功');
      } 
    })
  },
  connectmqtt:function(){
    var that=this
    const options={
      connectTimeout:4000,
      clientId:'wxmp'+Math.ceil(Math.random()*10),
      port:8084,
      username:'fc70ac3fa5e8792e875f23c7a88b5f4b',
      password:'123123'
    }
    client=mqtt.connect('wxs://t.yoyolife.fun/mqtt',options)
    client.on('connect',(e)=>{
      console.log('服务器连接成功')
    })
  }
})
