const EventEmitter=require('events');
class MyEm extends EventEmitter{}
function boo(){console.log("an ev occured");}
const m=new MyEm();
m.on('ev', boo)
m.on('ev', boo)
m.emit('ev',null)
m.removeListener('ev', boo);
console.log("***")
m.emit('ev', null)
m.removeListener('ev', boo)
console.log("***")
m.emit('ev', null)
