// index.js
var mqtt = require('../../utils/mqtt.min.js');
var client=null
Page({
  data: {
    temp:0,
    heart:0,
    blood:0,
    weight:0,
    height:175,
    BMI:0,
  },
  onLoad(){
    this.connectmqtt()
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
      client.subscribe('/iot/5898/pub/1', {qos: 0},  function(err){
        if (!err) {
          console.log('subscribe message successful');
        } 
      })
    })
    //信息监听
    client.on('message',function(topic,message){
      let recv={}
      recv=JSON.parse(message)
      console.log(recv)
      that.setData({
        temp:recv.services[0].properties.Temperature,
        heart:recv.services[0].properties.HeartRate,
        blood:recv.services[0].properties.BloodOxygen,
        //height:recv.services[0].properties.Height,
        BMI:recv.services[0].properties.BMI,
        weight:recv.services[0].properties.Weight
      })
      console.log('temp='+that.data.temp)
      console.log('heart='+that.data.heart)
      console.log('blood='+that.data.blood)
      console.log('weight='+that.data.weight)
      console.log('BMI='+that.data.BMI)
      console.log('收到'+message.toString())
    })
    client.on('reconnect',(error)=>{
      console.log('正在重连',error)
    })
    client.on('error',(error)=>{
      console.log('连接失败',error)
    })
  },
  bindKeyInput: function (e) {
    this.setData({
      height: e.detail.value
    })
    console.log(this.data.height)
  },
  setHeight:function(){
    var that=this
    client.publish('/iot/5898/sub/1', ' { "service_id": "Health_System", "command_name": "Height", "paras": { "height": '+that.data.height+' } }',  function(err){
      if (!err) {
        console.log('发送成功');
      } 
    })
    // client.subscribe('/iot/5898/pub/1', {qos: 0},  function(err){
    //   if (!err) {
    //     console.log('subscribe message successful');
    //   } 
    // })
  },
})
