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
let jobid= await b.publish(jobname,{param:'param1'},{startAfter:'8 seconds'});
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

boss.subscribe(jobname,{newJobCheckIntervalSeconds:10000},(job)=>{
console.log('sub ',job.name,'\n',job.id,'\n',job.data);	
//done().then(()=>{console.log('confirmed done.')})
job.done("fucking error");
}).then(()=>{console.log("subscription created!")}).catch(err=>{console.log('err3: ',err);})	

/*
boss.subscribe('jobname',{newJobCheckIntervalSeconds:4},(job)=>{
console.log('sub ',job.name,'\n',job.id,'\n',job.data);	
//done().then(()=>{console.log('confirmed done.')})
job.done("fucking error");
}).then(()=>{console.log("subscription created!")}).catch(err=>{console.log('err3: ',err);})	
*/


}
