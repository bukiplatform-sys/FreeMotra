import config from './config.js';
const $=id=>document.getElementById(id),fields=['name','titleTr','titleEn','bodyTr','bodyEn','link','start','end'];
let token=sessionStorage.getItem('freemotra-admin-token')||'',items=[],index=0,revision=0;
const notify=text=>$('status').textContent=text;
function leave(){token='';sessionStorage.removeItem('freemotra-admin-token');items=[];$('workspace').hidden=true;$('login').hidden=false;$('logout').hidden=true;}
async function api(path,method='GET',body){
 const r=await fetch(config.api+'/api/admin/'+path,{method,headers:{'Content-Type':'application/json',...(token?{Authorization:'Bearer '+token}:{})},body:body===undefined?undefined:JSON.stringify(body)});
 let data;try{data=await r.json();}catch{throw Error('Sunucudan yanıt alınamadı.');}
 if(!r.ok){if(r.status===401&&path!=='login'&&path!=='password')leave();throw Error(data.error||'İşlem tamamlanamadı.');}return data;
}
function pull(){const s=items[index];if(!s)return;for(const f of fields){const value=$('editor').elements[f].value;s[f]=['start','end'].includes(f)&&value?new Date(value).toISOString():value;}s.active=$('editor').elements.active.checked;preview();}
function preview(){const s=items[index];if(!s)return;$('preview-name').textContent=s.name;$('preview-title').textContent=s.titleTr;$('preview-body').textContent=s.bodyTr;$('preview-image').hidden=!s.image;if(s.image&&s.image.startsWith(config.projectUrl+'/storage/v1/object/public/freemotra-sponsor-images/'))$('preview-image').src=s.image;else $('preview-image').removeAttribute('src');}
function render(){
 const list=$('sponsor-list');list.replaceChildren();items.forEach((s,i)=>{const o=document.createElement('option');o.value=i;o.textContent=(s.active?'● ':'○ ')+(s.name||'Yeni reklam');list.append(o);});list.value=index;
 const s=items[index];$('editor').hidden=!s;$('delete').disabled=!s;
 if(s){for(const f of fields){let value=s[f];if(['start','end'].includes(f)&&value){const d=new Date(value);value=new Date(d.getTime()-d.getTimezoneOffset()*60000).toISOString().slice(0,16);}$('editor').elements[f].value=value;}$('editor').elements.active.checked=s.active;preview();}
 $('connection-state').textContent='Supabase bağlantısı açık · '+items.length+'/20 reklam · Yayın sürümü '+revision;
}
async function reload(){const r=await api('sponsors');items=r.sponsors;revision=Number(r.revision);index=Math.min(index,Math.max(0,items.length-1));render();}
async function enter(){await api('session');await reload();await loadNetwork();$('workspace').hidden=false;$('login').hidden=true;$('logout').hidden=false;}
async function publish(){pull();const r=await api('sponsors','PUT',{sponsors:items,revision});revision=Number(r.revision);render();notify('Yayın güncellendi. Güncel FreeMotra uygulamaları reklamları sonraki yenilemede alır.');}
$('login-form').onsubmit=async e=>{e.preventDefault();const button=e.target.querySelector('button');button.disabled=true;try{const r=await api('login','POST',{password:$('password').value});$('password').value='';token=r.token;sessionStorage.setItem('freemotra-admin-token',token);await enter();notify('Giriş yapıldı.');}catch(e){notify(e.message);}finally{button.disabled=false;}};
$('new').onclick=()=>{pull();if(items.length>=20)return notify('En fazla 20 reklam.');items.push({id:crypto.randomUUID(),name:'Yeni sponsor',titleTr:'Yeni başlık',titleEn:'',bodyTr:'',bodyEn:'',link:'',image:'',start:'',end:'',active:false});index=items.length-1;render();notify('Reklam taslağı eklendi.');};
$('sponsor-list').onchange=e=>{pull();index=Number(e.target.value);render();};
$('delete').onclick=()=>{if(!items[index])return;items.splice(index,1);index=Math.max(0,index-1);render();notify('Listeden kaldırıldı. Kalıcı olması için listeyi yayınlayın.');};
$('reload').onclick=async()=>{try{await reload();notify('Yayın listesi yenilendi.');}catch(e){notify(e.message);}};
$('editor').oninput=pull;$('editor').onsubmit=async e=>{e.preventDefault();try{await publish();}catch(e){notify(e.message);}};
const saveAll=document.createElement('button');saveAll.textContent='Tüm listeyi yayınla';saveAll.onclick=async()=>{try{await publish();}catch(e){notify(e.message);}};document.querySelector('.toolbar').append(saveAll);
$('logout').onclick=async()=>{try{await api('logout','POST',{});}catch{}finally{leave();notify('Çıkış yapıldı.');}};
$('image-upload').onchange=async e=>{const f=e.target.files[0];if(!f||!items[index])return;if(f.size>1000000||f.type!=='image/png')return notify('En fazla 1 MB PNG seçin.');try{const bytes=new Uint8Array(await f.arrayBuffer());let str='';for(const byte of bytes)str+=String.fromCharCode(byte);const r=await api('image','POST',{png:btoa(str)});items[index].image=r.image;preview();notify('Görsel yüklendi. Kullanıcılara göndermek için yayınlayın.');}catch(e){notify(e.message);}e.target.value='';};
$('clear-image').onclick=()=>{if(items[index])items[index].image='';preview();};
$('password-form').onsubmit=async e=>{e.preventDefault();try{await api('password','POST',{current:$('current-password').value,password:$('new-password').value});e.target.reset();leave();notify('Şifre değiştirildi; tüm yönetici oturumları kapatıldı. Yeni şifreyle giriş yapın.');}catch(e){notify(e.message);}};
if(token)enter().catch(()=>leave());

async function loadNetwork(){const r=await api('network');$('relay-enabled').checked=r.enabled;$('relay-mode').value=r.mode;$('relay-urls').value=r.urls.join('\n');$('relay-username').value=r.username;$('relay-secret').value='';$('relay-state').textContent=r.enabled?'Röle etkin. Yeni bağlantılar bu ayarı kullanır.':r.hasSecret?'Röle kapalı; kimlik bilgileri kayıtlı.':'Röle tanımlı değil. Uygulama doğrudan bağlantıyı kullanır.';}
$('network-form').onsubmit=async e=>{e.preventDefault();const button=e.target.querySelector('button');button.disabled=true;try{await api('network','PUT',{enabled:$('relay-enabled').checked,mode:$('relay-mode').value,urls:$('relay-urls').value.split(/\r?\n/).map(s=>s.trim()).filter(Boolean),username:$('relay-username').value.trim(),secret:$('relay-secret').value});await loadNetwork();notify('İnternet rölesi ayarı kaydedildi. Yeni bağlantılarda uygulanır.');}catch(e){notify(e.message);}finally{button.disabled=false;}};
