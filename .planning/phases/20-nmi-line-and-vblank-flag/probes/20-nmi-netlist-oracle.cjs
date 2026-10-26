const vm=require('node:vm'),crypto=require('node:crypto'),fs=require('node:fs');
const sources=JSON.parse(fs.readFileSync(0,'utf8'));
const files=['segdefs.js','transdefs.js','nodenames.js','wires.js','chipsim.js','macros.js'];
for(const f of files)console.log('SOURCE',f,crypto.createHash('sha256').update(sources[f]).digest('hex'));
function create(){
 const c=vm.createContext({console,Date,alert:m=>{throw Error(m)},document:{getElementById:()=>({getContext:()=>({clearRect(){}}),width:1,height:1})}});
 for(const f of files)vm.runInContext(sources[f],c,{filename:f});
 vm.runInContext(`now=()=>Date.now();refresh=()=>{};chipStatus=()=>{};setCellValue=()=>{};setupNodes();setupTransistors();
 memory=Array(65536).fill(0xea);
 memory[0x8000]=0xea;memory[0x8001]=0xea;memory[0x8002]=0x4c;memory[0x8003]=0;memory[0x8004]=0x80;
 memory[0x9000]=0xe6;memory[0x9001]=2;memory[0x9002]=0x40;memory[2]=0;
 memory[0xfffa]=0;memory[0xfffb]=0x90;memory[0xfffc]=0;memory[0xfffd]=0x80;
 initChip();var tick=0;
 function masterHalf(){var old=readBit('clk0');if(readBit('clk_in'))setLow('clk_in');else setHigh('clk_in');tick++;if(old!=readBit('clk0')){if(old)handleBusRead();else handleBusWrite();}}
 function snap(label){return [label,tick,readBit('clk_in'),readBit('clk0'),+isNodeHigh(710),readBit('cclk'),readBit('phi2'),readBit('nmi'),readBit('NMIP'),readBit('~NMIG'),readPC().toString(16),memory[2]].join('\\t');}
 for(var i=0;i<2400;i++)masterHalf();`,c);
 return c;
}
console.log('label\tmaster_half\tmaster_in\tclk0\tphi1_710\tinternal_phi2_943\tM2\tnmi_pad\tNMIP\t~NMIG\tPC\thandler_count');
const cases=[['pre_fall',2424,2432,1],['post_fall_short',2432,2436,0],['phi2_short',2420,2424,0],['M2_early_only',2438,2439,0],['post_fall_long',2432,2456,1],['no_pulse',-1,-1,0]];
for(const [name,low,high,want]of cases){
 const c=create();console.log('CASE',name,'low',low,'high',high,'expected_handler_count',want);
 console.log(vm.runInContext('snap("baseline")',c));
 for(let t=2401;t<=2500;t++){
  vm.runInContext('masterHalf()',c);
  console.log(vm.runInContext('snap("clock")',c));
  if(t===low){vm.runInContext('setLow("nmi")',c);console.log(vm.runInContext('snap("set_low")',c));}
  if(t===high){vm.runInContext('setHigh("nmi")',c);console.log(vm.runInContext('snap("set_high")',c));}
 }
 vm.runInContext('for(var i=0;i<1200;i++)masterHalf()',c);
 console.log(vm.runInContext('snap("final")',c));
 const got=vm.runInContext('memory[2]',c);if(got!==want)throw Error(name+': expected '+want+' got '+got);
}
console.log('PASS six independent fresh-state pulse cases; all prescribed pulses avoid exact clock ties');
