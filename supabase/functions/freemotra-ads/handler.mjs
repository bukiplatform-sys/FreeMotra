const encoder=new TextEncoder();
const hex=bytes=>Array.from(new Uint8Array(bytes),v=>v.toString(16).padStart(2,'0')).join('');
export async function sha256(value){return hex(await crypto.subtle.digest('SHA-256',typeof value==='string'?encoder.encode(value):value));}
export async function derivePassword(password,salt,iterations=310000){
 const key=await crypto.subtle.importKey('raw',encoder.encode(password),'PBKDF2',false,['deriveBits']);
 return hex(await crypto.subtle.deriveBits({name:'PBKDF2',hash:'SHA-256',salt:encoder.encode(salt),iterations},key,256));
}
const equal=(a,b)=>{if(typeof a!=='string'||typeof b!=='string'||a.length!==b.length)return false;let diff=0;for(let i=0;i<a.length;i++)diff|=a.charCodeAt(i)^b.charCodeAt(i);return diff===0;};
export function validateSponsors(input,imagePrefix){
 if(!Array.isArray(input)||input.length>20)throw Error('En fazla 20 reklam olabilir.');
 const ids=new Set();return input.map(s=>{
  if(!s||typeof s!=='object')throw Error('Geçersiz reklam.');
  const out={};for(const [key,max] of Object.entries({id:40,name:70,titleTr:160,titleEn:160,bodyTr:350,bodyEn:350,link:1024,image:1024,start:24,end:24})){
   if(typeof s[key]!=='string'||s[key].length>max)throw Error('Geçersiz alan: '+key);out[key]=s[key].trim();
  }
  if(!/^[a-zA-Z0-9_-]{1,40}$/.test(out.id)||ids.has(out.id))throw Error('Reklam kimliği benzersiz olmalı.');ids.add(out.id);
  if(!out.name||!out.titleTr)throw Error('Sponsor adı ve Türkçe başlık gerekli.');
  if(out.link){const u=new URL(out.link);if(u.protocol!=='https:'||u.username||u.password)throw Error('Bağlantı HTTPS olmalı.');}
  if(out.image){const tail=out.image.slice(imagePrefix.length);if(!out.image.startsWith(imagePrefix)||!/^([a-f0-9]{64})\.png$/.test(tail))throw Error('Görseli bu panelden yükleyin.');}
  for(const key of ['start','end'])if(out[key]&&!Number.isFinite(Date.parse(out[key])))throw Error('Tarih geçersiz.');
  if(out.start&&out.end&&Date.parse(out.start)>=Date.parse(out.end))throw Error('Bitiş başlangıçtan sonra olmalı.');
  out.active=s.active===true;return out;
 });
}
export function createAdsHandler({url,secret,origin,fetcher=fetch,now=()=>Date.now()}){
 const storagePrefix=url+'/storage/v1/object/public/freemotra-sponsor-images/';
 const headers={apikey:secret,...(secret.startsWith('eyJ')?{Authorization:'Bearer '+secret}:{})};
 async function db(path,method='GET',body,extra={}){
  const r=await fetcher(url+'/rest/v1/'+path,{method,headers:{...headers,'Content-Type':'application/json',...extra},body:body===undefined?undefined:JSON.stringify(body)});
  const text=await r.text();let data;try{data=text?JSON.parse(text):null;}catch{throw Error('Veritabanı yanıtı geçersiz.');}
  if(!r.ok){console.error('FreeMotra database operation failed',path.split('?')[0],r.status,data?.code,data?.message);const e=Error(['40001','PT409'].includes(data?.code)?'Yayın başka bir pencerede değişti. Listeyi yenileyin.':'Veri işlemi tamamlanamadı.');e.status=['40001','PT409'].includes(data?.code)?409:500;throw e;}return data;
 }
 return async req=>{
  const allowed=req.headers.get('Origin')===origin;
  const cors=allowed?{'Access-Control-Allow-Origin':origin,'Vary':'Origin','Access-Control-Allow-Methods':'GET,POST,PUT,OPTIONS','Access-Control-Allow-Headers':'Authorization,Content-Type'}:{};
  const respond=(status,body,extra={})=>new Response(JSON.stringify(body),{status,headers:{'Content-Type':'application/json; charset=utf-8','Cache-Control':'no-store','X-Content-Type-Options':'nosniff',...cors,...extra}});
  const cleanPath=new URL(req.url).pathname;const marker='/freemotra-ads';const markerAt=cleanPath.indexOf(marker);const path=markerAt>=0?cleanPath.slice(markerAt+marker.length):cleanPath;
  if(req.method==='OPTIONS')return allowed?new Response(null,{status:204,headers:cors}):respond(403,{error:'İzin verilmeyen kaynak.'});
  if(req.headers.has('Origin')&&!allowed)return respond(403,{error:'İzin verilmeyen kaynak.'});
  if(path==='/health'&&req.method==='GET')return respond(200,{ok:true,service:'FreeMotra Sponsor Studio'});
  if(!path.startsWith('/api/admin/'))return respond(404,{error:'Bulunamadı.'});
  async function body(limit=64000){
   const length=Number(req.headers.get('Content-Length')||0);if(length>limit)throw Error('İstek çok büyük.');
   const reader=req.body?.getReader();if(!reader)throw Error('İstek gövdesi gerekli.');
   let size=0;const chunks=[];while(true){const {done,value}=await reader.read();if(done)break;size+=value.length;if(size>limit){await reader.cancel();throw Error('İstek çok büyük.');}chunks.push(value);}
   const bytes=new Uint8Array(size);let offset=0;for(const chunk of chunks){bytes.set(chunk,offset);offset+=chunk.length;}return JSON.parse(new TextDecoder().decode(bytes));
  }
  try{
   if(path==='/api/admin/login'&&req.method==='POST'){
    const ip=(req.headers.get('x-forwarded-for')||'unknown').split(',')[0].trim();
    const [attempts,globalAttempts]=await Promise.all([db('rpc/freemotra_login_attempt','POST',{attempt_key:'ip:'+await sha256(ip)}),db('rpc/freemotra_login_attempt','POST',{attempt_key:'global'})]);
    if(attempts>5||globalAttempts>100)return respond(429,{error:'Çok fazla giriş denemesi. 15 dakika sonra tekrar deneyin.'});
    const input=await body(2048);if(typeof input.password!=='string'||input.password.length>128)return respond(401,{error:'Şifre yanlış.'});
    const settings=(await db('freemotra_admin_settings?id=eq.true&select=password_salt,password_hash,iterations'))[0];
    if(!settings||!equal(await derivePassword(input.password,settings.password_salt,settings.iterations),settings.password_hash))return respond(401,{error:'Şifre yanlış.'});
    await db('freemotra_admin_sessions?expires_at=lt.'+encodeURIComponent(new Date(now()).toISOString()),'DELETE');
    const token=hex(crypto.getRandomValues(new Uint8Array(32)));
    await db('freemotra_admin_sessions','POST',{token_hash:await sha256(token),expires_at:new Date(now()+1800000).toISOString()});
    await db('freemotra_login_limits?key=eq.'+encodeURIComponent('ip:'+await sha256(ip)),'DELETE');
    return respond(200,{token,expiresAt:now()+1800000});
   }
   const auth=req.headers.get('Authorization')||'';const token=auth.startsWith('Bearer ')?auth.slice(7):'';
   if(!/^[a-f0-9]{64}$/.test(token))return respond(401,{error:'Yönetici girişi gerekli.'});
   const tokenHash=await sha256(token);const session=await db('freemotra_admin_sessions?token_hash=eq.'+tokenHash+'&expires_at=gt.'+encodeURIComponent(new Date(now()).toISOString())+'&select=token_hash');
   if(!session.length)return respond(401,{error:'Oturum süresi doldu. Yeniden giriş yapın.'});
   if(path==='/api/admin/session'&&req.method==='GET')return respond(200,{ok:true});
   if(path==='/api/admin/logout'&&req.method==='POST'){await db('freemotra_admin_sessions?token_hash=eq.'+tokenHash,'DELETE');return respond(200,{ok:true});}
   if(path==='/api/admin/sponsors'&&req.method==='GET'){
    const campaign=(await db('freemotra_campaigns?id=eq.true&select=revision,sponsors,updated_at'))[0];return respond(200,campaign);
   }
   if(path==='/api/admin/sponsors'&&req.method==='PUT'){
    const input=await body();if(!Number.isSafeInteger(input.revision)||input.revision<0)throw Error('Yayın sürümü gerekli.');
    const sponsors=validateSponsors(input.sponsors,storagePrefix);
    const revision=await db('rpc/freemotra_publish','POST',{sponsor_list:sponsors,expected_revision:input.revision});return respond(200,{ok:true,revision});
   }
   if(path==='/api/admin/image'&&req.method==='POST'){
    const input=await body(1400000);if(typeof input.png!=='string'||input.png.length>1333400)throw Error('PNG görseli en fazla 1 MB olabilir.');
    let bytes;try{bytes=Uint8Array.from(atob(input.png),c=>c.charCodeAt(0));}catch{throw Error('Geçersiz görsel.');}
    const view=new DataView(bytes.buffer),signature=[137,80,78,71,13,10,26,10];
    if(bytes.length<33||bytes.length>1000000||!signature.every((n,i)=>bytes[i]===n)||String.fromCharCode(...bytes.slice(12,16))!=='IHDR'||view.getUint32(16)<1||view.getUint32(20)<1||view.getUint32(16)>1600||view.getUint32(20)>1600)throw Error('Geçerli PNG gerekli (en fazla 1600×1600).');
    const name=await sha256(bytes)+'.png';
    const uploaded=await fetcher(url+'/storage/v1/object/freemotra-sponsor-images/'+name,{method:'POST',headers:{...headers,'Content-Type':'image/png','Cache-Control':'max-age=31536000','x-upsert':'true'},body:bytes});
    if(!uploaded.ok)throw Error('Görsel yüklenemedi.');return respond(200,{image:storagePrefix+name});
   }
   if(path==='/api/admin/password'&&req.method==='POST'){
    const input=await body(2048);if(typeof input.current!=='string'||input.current.length>128||typeof input.password!=='string'||input.password.length<12||input.password.length>128)throw Error('Yeni şifre 12–128 karakter olmalı.');
    const settings=(await db('freemotra_admin_settings?id=eq.true&select=password_salt,password_hash,iterations'))[0];
    if(!equal(await derivePassword(input.current,settings.password_salt,settings.iterations),settings.password_hash))return respond(401,{error:'Mevcut şifre yanlış.'});
    const salt=hex(crypto.getRandomValues(new Uint8Array(16))),hash=await derivePassword(input.password,salt);
    await db('freemotra_admin_settings?id=eq.true','PATCH',{password_salt:salt,password_hash:hash,iterations:310000});
    await db('freemotra_admin_sessions?token_hash=not.is.null','DELETE');return respond(200,{ok:true});
   }
   return respond(405,{error:'İşlem desteklenmiyor.'});
  }catch(error){return respond(error.status||400,{error:error.status===500?'Sunucuya ulaşılamadı. Tekrar deneyin.':error.message||'İstek işlenemedi.'});}
 };
}

