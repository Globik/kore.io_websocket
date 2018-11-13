const EventEmitter=require('events');
class MyEm extends EventEmitter{}
const m=new MyEm();
m.on('ev',function(){console.log("an ev occured");})
m.on('ev', function(){console.log("an ev occured");})
m.emit('ev',null)
