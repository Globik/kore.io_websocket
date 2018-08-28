const port=8888;
const dburl="postgress://globik:null@localhost:5432/postgres";
const Koa=require('koa');
const PgBoss=require('pg-boss');
const Router=require('koa-router');
const srouter=new Router();
const jobname='jobbi';
const boss=new PgBoss(dburl);
boss.on('error',function(error){console.log('pg boss err: ',error)})

const app=new Koa();

app.use(async (ctx,next)=>{
ctx.boss=boss;
await next();	
})

srouter.get('/',async ctx=>{
let b=ctx.boss;
try{
let jobid= await b.publish(jobname,{param:'Chicago'},{startAfter:'10 seconds'});
console.log('created job from request: ', jobname,':',jobid);
}catch(e){console.log('err in req: ',e)}
ctx.body={"info":"info","page":"/"}
})

app.use(srouter.routes()).use(srouter.allowedMethods());
const servak=app.listen(port);
console.log('localhost: ',port);

boss.start().then(ready).catch(err=>console.log('boss start err: ',err))

function ready(){
console.log("pg boss is ready!");

boss.subscribe(jobname,{newJobCheckIntervalSeconds:1000},(job)=>{
console.log('Subcriber[name, id, data]: ',job.name,'\n',job.id,'\n',job.data);	

//var a=job.done("o no, Fucker!").then(function(val){
var a=job.done(null,"some success info").then(function(val){
//console.log("*** val_A: ***",val)
	console.log("****************************");
	console.log("val.jobs[0]: ",val.jobs[0]);
	console.log("val.requested: ",val.requested);
	console.log("val.updated: ",val.updated);
	console.log("^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
}).catch(function(er){console.log("err_A: ",er)});

console.log("*** A? ***",a);//should be a Promise

}).then((vali)=>{console.log("subscription created!",vali)}).catch(err=>{console.log('err3: ',err);})	

/*
boss.subscribe('jobname',{newJobCheckIntervalSeconds:4},(job)=>{
console.log('sub ',job.name,'\n',job.id,'\n',job.data);	
//done().then(()=>{console.log('confirmed done.')})
job.done("fucking error");
}).then(()=>{console.log("subscription created!")}).catch(err=>{console.log('err3: ',err);})	
*/


}
