// firts run janus.exe
// then run node unix.js
const net=require('net');
//home/globik/ux-janusapi
/*
const client=net.createConnection({path:'/home/globik/ux-janusapi'},()=>{
console.log("connected to janus phunix");
client.write("world!\r\n");
});
*/

const client=net.createConnection({path:'/home/globik/fuck'});
client.on('connect',()=>{console.log("conn");});
client.on('data',(data)=>{
console.log(data.toString());
client.end();
});

client.on('end',()=>{console.log('disconnected from janus phunix');});
client.on('error',(e)=>{console.log('some err: ',e);});


//var serv=net.createServer((s)=>{console.log("fo");});
//serv.listen('/home/globik/ux-janudsapi.sock');