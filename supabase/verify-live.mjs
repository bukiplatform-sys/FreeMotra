// Private integration run. Reads the administrator password from hidden stdin only.
import {spawnSync} from 'node:child_process';
import assert from 'node:assert/strict';
const url='https://mesqqxlacjuvinsdospy.supabase.co',origin='https://freemotra-sponsor-studio.sonatacello.chatgpt.site';
const key='sb_publishable_UIWN_Ypc9N2hVi9YK9tR8Q_e8zCAm-n';
const quote=value=>'"'+String(value).replaceAll('\\','\\\\').replaceAll('"','\\"').replaceAll('\n','\\n')+'"';
function request(path,method='GET',body,headers={}){
 const config=['silent','show-error','ssl-revoke-best-effort','max-time = 30','url = '+quote(url+path),'request = '+quote(method),'write-out = "\\n%{http_code}"',...Object.entries(headers).map(([k,v])=>'header = '+quote(k+': '+v))];
 if(body!==undefined){config.push('header = "Content-Type: application/json"','data-binary = '+quote(JSON.stringify(body)));}
 const r=spawnSync('curl.exe',['--config','-'],{input:config.join('\n'),encoding:'utf8',maxBuffer:5000000,windowsHide:true});
 if(r.status!==0)throw Error('HTTPS request failed: '+r.stderr);
 const pos=r.stdout.lastIndexOf('\n'),status=Number(r.stdout.slice(pos+1)),raw=r.stdout.slice(0,pos);let data;try{data=JSON.parse(raw);}catch{data=null;}return {status,data};
}
function readInput(){return new Promise((resolve,reject)=>{let text='';if(process.stdin.isTTY){process.stdin.setRawMode(true);console.error('Ready for private verification JSON on stdin (input hidden).');}process.stdin.setEncoding('utf8');process.stdin.on('data',data=>{text+=data;if(text.includes('\n')||text.includes('\r')){if(process.stdin.isTTY)process.stdin.setRawMode(false);process.stdin.pause();try{resolve(JSON.parse(text));}catch(e){reject(e);}}});process.stdin.resume();});}
const {password}=await readInput();let token='',revision=0,original=[],changed=false;
const api=(path,method='GET',body)=>request('/functions/v1/freemotra-ads/api/admin/'+path,method,body,{Origin:origin,...(token?{Authorization:'Bearer '+token}:{})});
try{
 assert.equal(request('/functions/v1/freemotra-ads/health').status,200);
 assert.equal(api('sponsors').status,401);
 const logged=api('login','POST',{password});assert.equal(logged.status,200,JSON.stringify(logged.data));token=logged.data.token;
 const initial=api('sponsors');assert.equal(initial.status,200);original=initial.data.sponsors;revision=Number(initial.data.revision);
 assert.equal(request('/rest/v1/freemotra_admin_settings?select=*','GET',undefined,{apikey:key}).status,401);
 const ad={id:'connection-test',name:'FreeMotra test',titleTr:'Connection test',titleEn:'',bodyTr:'',bodyEn:'',link:'',image:'',start:'',end:'',active:true};
 assert.ok([401,403].includes(request('/rest/v1/freemotra_ads','POST',{id:'unauthorized'},{apikey:key}).status));
 const published=api('sponsors','PUT',{revision,sponsors:[ad,{...ad,id:'draft-test',active:false},{...ad,id:'future-test',start:new Date(Date.now()+86400000).toISOString()}]});
 assert.equal(published.status,200,JSON.stringify(published.data));revision=Number(published.data.revision);changed=true;
 const rows=request('/rest/v1/freemotra_ads?select=payload&order=position.asc','GET',undefined,{apikey:key});assert.equal(rows.status,200);assert.equal(rows.data.length,1);assert.equal(rows.data[0].payload.id,'connection-test');
 assert.equal(api('sponsors','PUT',{revision:revision-1,sponsors:[]}).status,409);
 const png='iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAusB9Wl6nmcAAAAASUVORK5CYII=';
 const uploaded=api('image','POST',{png});assert.equal(uploaded.status,200,JSON.stringify(uploaded.data));assert.ok(uploaded.data.image.startsWith(url+'/storage/v1/object/public/freemotra-sponsor-images/'));
 const restored=api('sponsors','PUT',{revision,sponsors:original});assert.equal(restored.status,200);revision=Number(restored.data.revision);changed=false;
 assert.equal(api('logout','POST',{}).status,200);assert.equal(api('session').status,401);
 console.log(JSON.stringify({ok:true,checks:['health','administrator login','anonymous administration denied','anonymous writes denied','published ads visible','draft and future ads hidden','revision conflict','PNG storage upload','original campaign restored','logout revocation']}));
}finally{if(changed&&token){const restored=api('sponsors','PUT',{revision,sponsors:original});console.log(JSON.stringify({cleanup:restored.status===200}));}}
