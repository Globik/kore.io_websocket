const EventEmitter=require('events');
var plans={
	purge(){return Promise.resolve("some data");}
	}
class Boss extends EventEmitter{
	constructor(){
	super();
	this.timers={};
	this.purge_command=plans.purge();	
	}
	
	supervise(){
		const self=this;
		return Promise.all([monitor(this.purge,3000)]);
		
		
		function monitor(func,interval){
		return exec().then(repeat);
		
		function exec(){console.log("first commit?");
			return func.call(self).catch(err=>{console.log('era: ',err)});}
		function repeat(){
			console.log("repeat? ", func.name);
			//console.log("in timers: ",self.timers.purge);
			self.timers[func.name]=setTimeout(()=>{console.log("timeout");exec().then(repeat)},interval);}
		
	}
}
purge(){
return plans.purge().then(function(bu){console.log('bu: ',bu)});
}
}

var b=new Boss();
b.supervise().then(function(d){console.log('d',d)});
	function foo(){
		console.log("foo")
		setTimeout(function(){console.log("foo1");foo();},1000);
		}
		foo();
