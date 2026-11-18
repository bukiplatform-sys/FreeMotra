import http from 'node:http';
import {randomBytes,scryptSync,timingSafeEqual,createHash} from 'node:crypto';
import {mkdir,readFile,writeFile,rename} from 'node:fs/promises';
import {fileURLToPath} from 'node:url';
import {join} from 'node:path';
const digest=s=>createHash('sha256').update(s).digest('hex');
const safeEqual=(a,b)=>a.length===b.length&&timingSafeEqual(a,b);
export function passwordHash(password){let salt=randomBytes(16);return salt.toString('hex')+':'+scryptSync(password,salt,64).toString('hex');}
export function validateSponsors(input){
 if(!Array.isArray(input)||input.length>20)throw Error('En fazla 20 sponsor olabilir.');
 const ids=new Set();return input.map(s=>{
  if(!s||typeof s!=='object')throw Error('Geçersiz sponsor.');
  const out={};for(const [key,max] of Object.entries({id:40,name:70,titleTr:160,titleEn:160,bodyTr:350,bodyEn:350,link:1024,image:100,start:24,end:24})){
   if(typeof s[key]!=='string'||s[key].length>max)throw Error('Geçersiz alan: '+key);out[key]=s[key].trim();
  }
  if(!/^[a-zA-Z0-9_-]{1,40}$/.test(out.id)||ids.has(out.id))throw Error('Sponsor kimliği benzersiz olmalı.');ids.add(out.id);
  if(!out.name||!out.titleTr)throw Error('Sponsor adı ve Türkçe başlık gerekli.');
  if(out.link){let u=new URL(out.link);if(u.protocol!=='https:'||u.username||u.password)throw Error('Bağlantı HTTPS olmalı.');}
  if(out.image&&!/^\/images\/[a-f0-9]{64}\.png$/.test(out.image))throw Error('Görseli panelden yükleyin.');
  for(const key of ['start','end'])if(out[key]&&!Number.isFinite(Date.parse(out[key])))throw Error('Tarih geçersiz.');
  if(out.start&&out.end&&Date.parse(out.start)>=Date.parse(out.end))throw Error('Bitiş başlangıçtan sonra olmalı.');
  out.active=s.active===true;return out;
 });
}
export async function createApp({directory,origin,password,secure=true}){
 const parsed=new URL(origin);if(secure&&parsed.protocol!=='https:')throw Error('Üretimde HTTPS origin gerekli.');
 if(!/^[a-f0-9]{32}:[a-f0-9]{128}$/.test(password))throw Error('ADMIN_PASSWORD_HASH scrypt salt:hash biçiminde olmalı.');
 const [salt,hash]=password.split(':').map(x=>Buffer.from(x,'hex'));
 await mkdir(join(directory,'images'),{recursive:true,mode:0o700});
 let sponsors=[];try{sponsors=validateSponsors(JSON.parse(await readFile(join(directory,'sponsors.json'),'utf8')));}catch(e){if(e.code!=='ENOENT')throw e;}
 const sessions=new Map(),attempts=new Map();let writing=Promise.resolve();
 const cookie=secure?'__Host-fm_admin':'fm_admin_dev';
 const publicDir=fileURLToPath(new URL('./public/',import.meta.url));
 const json=(res,status,data)=>{res.writeHead(status,{'Content-Type':'application/json; charset=utf-8','Cache-Control':'no-store'});res.end(JSON.stringify(data));};
 const body=async(req,max=70000)=>{let size=0,chunks=[];for await(const c of req){size+=c.length;if(size>max)throw Error('İstek çok büyük.');chunks.push(c);}return JSON.parse(Buffer.concat(chunks).toString('utf8'));};
 const cookieValue=(token,age)=>`${cookie}=${token}; HttpOnly; SameSite=Strict; Path=/; Max-Age=${age}${secure?'; Secure':''}`;
 async function handler(req,res){
  res.setHeader('X-Content-Type-Options','nosniff');res.setHeader('Referrer-Policy','no-referrer');res.setHeader('X-Frame-Options','DENY');
  res.setHeader('Content-Security-Policy',"default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' blob:; connect-src 'self'; frame-ancestors 'none'; base-uri 'none'; form-action 'self'");
  if(secure)res.setHeader('Strict-Transport-Security','max-age=31536000');
  const path=new URL(req.url,'http://local.invalid').pathname,now=Date.now();
  for(const [key,s]of sessions)if(now>s.expires)sessions.delete(key);
  for(const [key,s]of attempts)if(now>s.expires)attempts.delete(key);
  const token=(req.headers.cookie||'').split(';').map(x=>x.trim()).find(x=>x.startsWith(cookie+'='))?.slice(cookie.length+1)||'';
  const session=sessions.get(digest(token));
  if(req.method==='GET'&&path==='/api/sponsors'){
   const result=sponsors.filter(s=>s.active&&(!s.start||Date.parse(s.start)<=now)&&(!s.end||Date.parse(s.end)>now)).map(s=>({...s,image:s.image?origin+s.image:''}));
   return json(res,200,{version:1,sponsors:result});
  }
  if(req.method==='GET'&&/^\/images\/[a-f0-9]{64}\.png$/.test(path)){
   try{const file=await readFile(join(directory,path.slice(1)));res.writeHead(200,{'Content-Type':'image/png','Cache-Control':'public, max-age=86400'});res.end(file);}catch{json(res,404,{error:'Görsel bulunamadı.'});}return;
  }
  if(req.method==='GET'&&['/','/admin','/admin/','/app.js','/style.css'].includes(path)){
   const name=path==='/app.js'?'app.js':path==='/style.css'?'style.css':'index.html';
   res.writeHead(200,{'Content-Type':name.endsWith('.js')?'text/javascript':name.endsWith('.css')?'text/css':'text/html; charset=utf-8','Cache-Control':'no-store'});res.end(await readFile(join(publicDir,name)));return;
  }
  if(!path.startsWith('/api/admin/'))return json(res,404,{error:'Bulunamadı.'});
  if(req.method!=='GET'&&req.headers.origin!==origin)return json(res,403,{error:'Kaynak doğrulanamadı.'});
  if(path==='/api/admin/login'&&req.method==='POST'){
   const key=req.socket.remoteAddress||'unknown',state=attempts.get(key)||{count:0,expires:now+900000};
   if(state.count>=5||attempts.size>1000)return json(res,429,{error:'Çok fazla deneme. 15 dakika sonra tekrar deneyin.'});
   ++state.count;attempts.set(key,state);const input=await body(req,2048);
   if(typeof input.password!=='string'||input.password.length>256||!safeEqual(scryptSync(input.password,salt,64),hash))return json(res,401,{error:'Şifre yanlış.'});
   attempts.delete(key);if(sessions.size>=100)return json(res,429,{error:'Oturum sınırı.'});
   const raw=randomBytes(32).toString('hex'),csrf=randomBytes(32).toString('hex');sessions.set(digest(raw),{csrf,expires:now+1800000});
   res.setHeader('Set-Cookie',cookieValue(raw,1800));return json(res,200,{csrf});
  }
  if(!session)return json(res,401,{error:'Yönetici girişi gerekli.'});
  if(req.method!=='GET'&&req.headers['x-csrf-token']!==session.csrf)return json(res,403,{error:'Oturum doğrulaması başarısız.'});
  if(path==='/api/admin/session'&&req.method==='GET')return json(res,200,{csrf:session.csrf});
  if(path==='/api/admin/logout'&&req.method==='POST'){sessions.delete(digest(token));res.setHeader('Set-Cookie',cookieValue('',0));return json(res,200,{ok:true});}
  if(path==='/api/admin/sponsors'&&req.method==='GET')return json(res,200,{sponsors});
  if(path==='/api/admin/sponsors'&&req.method==='PUT'){
   const clean=validateSponsors((await body(req)).sponsors);
   const operation=writing.then(async()=>{const temp=join(directory,'sponsors.json.tmp');await writeFile(temp,JSON.stringify(clean,null,2),{mode:0o600});await rename(temp,join(directory,'sponsors.json'));sponsors=clean;});
   writing=operation.catch(()=>{});await operation;return json(res,200,{ok:true});
  }
  if(path==='/api/admin/image'&&req.method==='POST'){
   const input=await body(req,1500000);if(typeof input.png!=='string'||input.png.length>1400000)throw Error('PNG görseli en fazla 1 MB olabilir.');
   const file=Buffer.from(input.png,'base64');
   if(file.length<33||file.length>1000000||!file.subarray(0,8).equals(Buffer.from('89504e470d0a1a0a','hex'))||file.toString('ascii',12,16)!=='IHDR'||file.readUInt32BE(16)>1600||file.readUInt32BE(20)>1600||file.readUInt32BE(16)<1||file.readUInt32BE(20)<1)throw Error('Geçerli PNG gerekli (en fazla 1600×1600).');
   const name=digest(file)+'.png';await writeFile(join(directory,'images',name),file,{mode:0o600});return json(res,200,{image:'/images/'+name});
  }
  return json(res,405,{error:'İşlem desteklenmiyor.'});
 }
 return http.createServer((req,res)=>{handler(req,res).catch(()=>{if(!res.headersSent)json(res,400,{error:'İstek kaydedilemedi. Alanları ve sunucu izinlerini kontrol edin.'});else res.destroy();});}).on('clientError',(_err,socket)=>socket.end('HTTP/1.1 400 Bad Request\r\n\r\n'));
}
if(process.argv[1]===fileURLToPath(import.meta.url)){
 const server=await createApp({directory:process.env.DATA_DIR||'./data',origin:process.env.PUBLIC_ORIGIN||'http://127.0.0.1:3000',password:process.env.ADMIN_PASSWORD_HASH||'',secure:process.env.NODE_ENV==='production'});
 server.requestTimeout=10000;server.headersTimeout=10000;
 server.listen(Number(process.env.PORT||3000),process.env.HOST||'127.0.0.1',()=>console.log('FreeMotra sponsor yönetimi hazır.'));
}
