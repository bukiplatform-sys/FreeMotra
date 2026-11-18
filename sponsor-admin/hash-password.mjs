import {passwordHash} from './server.mjs';
let value='';for await(const chunk of process.stdin)value+=chunk;
value=value.trimEnd();if(value.length<16)throw Error('En az 16 karakterli güçlü şifre kullanın.');
console.log(passwordHash(value));
