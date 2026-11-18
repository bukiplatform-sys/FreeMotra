import test from 'node:test';
import assert from 'node:assert/strict';
import {createAdsHandler,derivePassword,validateSponsors} from './functions/freemotra-ads/handler.mjs';
const url='https://unit.supabase.co',origin='https://panel.example',prefix=url+'/storage/v1/object/public/freemotra-sponsor-images/';
const sponsor=(id='one')=>({id,name:'Sponsor',titleTr:'Baslik',titleEn:'',bodyTr:'',bodyEn:'',link:'https://example.com',image:'',start:'',end:'',active:true});
async function setup(){
 const password='unit-test-password-only',salt='unit-salt';let settings={password_salt:salt,password_hash:await derivePassword(password,salt),iterations:310000},campaign={revision:0,sponsors:[]};
 const sessions=new Map(),attempts=new Map();let uploads=0;
 const response=(data,status=200)=>new Response(JSON.stringify(data),{status,headers:{'Content-Type':'application/json'}});
 const handler=createAdsHandler({url,secret:'unit-server-secret',origin,fetcher:async(address,options)=>{
  const u=new URL(address),path=u.pathname.replace('/rest/v1/',''),method=options.method,body=options.body&&typeof options.body==='string'?JSON.parse(options.body):options.body;
  if(path==='rpc/freemotra_login_attempt'){const n=(attempts.get(body.attempt_key)||0)+1;attempts.set(body.attempt_key,n);return response(n);}
  if(path==='freemotra_admin_settings'){if(method==='PATCH'){settings={...settings,...body};return response(null);}return response([settings]);}
  if(path==='freemotra_admin_sessions'){
   const filter=u.searchParams.get('token_hash');
   if(method==='POST'){sessions.set(body.token_hash,body);return response(null);}
   if(method==='DELETE'){if(filter?.startsWith('eq.'))sessions.delete(filter.slice(3));else if(filter==='not.is.null')sessions.clear();return response(null);}
   const entry=sessions.get(filter?.slice(3));return response(entry&&Date.parse(entry.expires_at)>Date.now()?[entry]:[]);
  }
  if(path==='freemotra_login_limits')return response(null);
  if(path==='freemotra_campaigns')return response([campaign]);
  if(path==='rpc/freemotra_publish'){if(body.expected_revision!==campaign.revision)return response({code:'PT409'},409);campaign={revision:campaign.revision+1,sponsors:body.sponsor_list};return response(campaign.revision);}
  if(u.pathname.startsWith('/storage/v1/object/')){uploads++;return response({ok:true});}
  throw Error('Unexpected test request: '+path);
 }});
 const call=async(path,method='GET',body,token='',requestOrigin=origin)=>{const r=await handler(new Request(url+'/functions/v1/freemotra-ads/api/admin/'+path,{method,headers:{Origin:requestOrigin,'Content-Type':'application/json',...(token?{Authorization:'Bearer '+token}:{})},body:body===undefined?undefined:JSON.stringify(body)}));return {status:r.status,data:await r.json()};};
 const login=async()=>{const r=await call('login','POST',{password});assert.equal(r.status,200);return r.data.token;};
 return {call,login,password,get uploads(){return uploads;}};
}
test('rejects executable URLs, duplicate ids, foreign images and invalid schedules',()=>{
 assert.throws(()=>validateSponsors([{...sponsor(),link:'javascript:alert(1)'}],prefix));
 assert.throws(()=>validateSponsors([sponsor(),sponsor()],prefix));
 assert.throws(()=>validateSponsors([{...sponsor(),image:'https://evil.example/a.png'}],prefix));
 assert.throws(()=>validateSponsors([{...sponsor(),start:'2026-11-20T00:00:00Z',end:'2026-11-19T00:00:00Z'}],prefix));
 assert.equal(validateSponsors([sponsor()],prefix).length,1);
});
test('requires authentication and allowed origin; rate limit persists through requests',async()=>{
 const s=await setup();assert.equal((await s.call('sponsors')).status,401);assert.equal((await s.call('login','POST',{password:s.password},'','https://evil.example')).status,403);
 for(let i=0;i<5;i++)assert.equal((await s.call('login','POST',{password:'wrong'})).status,401);
 assert.equal((await s.call('login','POST',{password:s.password})).status,429);
});
test('publishes atomically, rejects stale revisions and permits an empty list',async()=>{
 const s=await setup(),token=await s.login();assert.equal((await s.call('sponsors','PUT',{revision:0,sponsors:[sponsor()]},token)).status,200);
 assert.equal((await s.call('sponsors','PUT',{revision:0,sponsors:[]},token)).status,409);
 const state=await s.call('sponsors','GET',undefined,token);assert.equal(state.data.sponsors.length,1);assert.equal(state.data.revision,1);
 assert.equal((await s.call('sponsors','PUT',{revision:1,sponsors:[]},token)).status,200);
 assert.equal((await s.call('sponsors','GET',undefined,token)).data.sponsors.length,0);
 await s.call('logout','POST',{},token);assert.equal((await s.call('session','GET',undefined,token)).status,401);
});
test('image validation and password change revoke existing sessions',async()=>{
 const s=await setup(),token=await s.login();assert.equal((await s.call('image','POST',{png:'not-png'},token)).status,400);assert.equal(s.uploads,0);
 assert.equal((await s.call('password','POST',{current:s.password,password:'changed-unit-password'},token)).status,200);
 assert.equal((await s.call('session','GET',undefined,token)).status,401);
 assert.equal((await s.call('login','POST',{password:s.password})).status,401);
 assert.equal((await s.call('login','POST',{password:'changed-unit-password'})).status,200);
});
