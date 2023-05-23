const fs = require('fs');

const PWD = process.cwd().replace('/mnt/c','C:');
const toolsPath = "C:/fd_work/TuKuRutchExe";

const getProp = function(obj, key) {
	return obj.hasOwnProperty(key) ? obj[key] : "";
}

const jsonToJs = function(target, ext)
{
	if(!ext.scratch3ext) return false;

	let define = "";
	let flashes = "";
	let _blocks = "";
	let menus = "";
	let funcs = "";
	let i;

	let extNames = ext.scratch3ext.split(",");
	define = "var extName = '" + extNames[0] + "';\n";
	if(extNames.length >= 2) {
		define += "const " + extNames[1] + " = true;\n";
	} else {
		define += "const SupportCamera = false;\n";
	}

	let copyFiles = "";
	for(i=0; i<ext.scratch3burn.length; i++) {
		let imageName = ext.scratch3burn[i].name;
		copyFiles += imageName+", ";

		//{name:'TukuBoard1.0', type:'esp32', baudrate:230400},

		flashes += "{name:'"+ext.scratch3burn[i].name
					+"', type:'"+ext.scratch3burn[i].type
					+"', baudrate:"+ext.scratch3burn[i].baudrate+"},\n";

		let binPath = ext.scratch3burn[i].binPath;
		let type = ext.scratch3burn[i].type;
		let src;
		switch(type) {
		case 'atmega328':
			src = binPath+"/src/src.ino.standard.hex";
			if(fs.existsSync(src))
				fs.copyFileSync(src, "../scratch3/"+imageName+".hex");
			break;

		case 'esp32c3u':
		case 'esp32s3u':
			type = type.slice(0,type.length-1);
		case 'esp32':
		case 'esp32c3':
		case 'esp32s3':
			src = binPath+"/src/src.ino."+type+".bin";
			if(fs.existsSync(src))
				fs.copyFileSync(src, "../scratch3/"+imageName+".image.bin");

			src = binPath+"/src/src.ino.bootloader.bin";
			if(fs.existsSync(src))
				fs.copyFileSync(src, "../scratch3/"+imageName+".boot.bin");

			src = binPath+"/src/src.ino.partitions.bin";
			if(fs.existsSync(src))
				fs.copyFileSync(src, "../scratch3/"+imageName+".part.bin");
			break;
		}
	}
	if(ext.scratch3burn.length==0) {
		flashes += "{name:'dummy', type:'', baudrate:0},\n";
	} else {
		//Main.app.scriptsPart.appendMessage("copy bin:"+copyFiles);
	}

	for(i=1; i<ext.blockSpecs.length; i++) {
//		["w", "set LED %d.led %d.onoff", "setLED", 1,"On", {"remote":["B","B"],	"func":"_setLED({0},{1});"}],

		let spec = ext.blockSpecs[i];
		if(spec.length < 3){
			if(spec[0] == "-") _blocks += "'---',\n";
			continue;
		}

		let txtEnOrg = spec[1];
		let txtEnNew = "";
		let txtJpOrg = "";
		let txtJpNew = "";
		let args = [];

		if(ext.hasOwnProperty("translators") && ext.translators.ja.hasOwnProperty(txtEnOrg))
			txtJpOrg = ext.translators.ja[txtEnOrg];
		
		let pos;
		let j;
		for(j = 0;;j++) {
			pos = txtEnOrg.indexOf("%");
			if(pos == -1) {
				txtEnNew += txtEnOrg;
				break;
			}
			txtEnNew += txtEnOrg.slice(0,pos) + "[ARG" + (j+1) + "]";
			txtEnOrg = txtEnOrg.slice(pos);

			pos = txtEnOrg.indexOf(" ");
			if(pos == -1) {
				args.push(txtEnOrg);
				txtEnOrg = "";
			} else {
				args.push(txtEnOrg.slice(0,pos));
				txtEnOrg = txtEnOrg.slice(pos);
			}
		}
		let argNum = j;
		for(j = 0;j < argNum;j++) {
			pos = txtJpOrg.indexOf(args[j]);
			if(pos == -1) {
				txtJpNew = "";
				break;
			}
			txtJpNew += txtJpOrg.slice(0,pos) + "[ARG" + (j+1) + "]";
			txtJpOrg = txtJpOrg.slice(pos + args[j].length);
		}
		if(j == 0) txtJpNew = txtJpOrg;
		else if(txtJpNew != "") txtJpNew += txtJpOrg;

		let obj = spec[spec.length-1];
		let types = [];
		if(obj.hasOwnProperty("enum")) {
			 types.push("B");
			funcs += spec[2] + "(args) { return args.ARG1; }\n";
		} else if(obj.hasOwnProperty("remote")) {
			types = obj["remote"];
			if(types.length < argNum)
				continue;
			funcs += spec[2] + "(args,util) { return this.sendRecv('" + spec[2] + "', args); }\n";
		} else if(obj.hasOwnProperty("custom")) {
			_blocks += "'---',\n";
			continue;
		} else {
			_blocks += "'---',\n";
			continue;
		}

		switch(spec[0]) {
		case "w":
			_blocks += "{blockType: BlockType.COMMAND, opcode: '" + spec[2] + "', text: ";
			break;
		case "R":
		case "r":
			_blocks += "{blockType: BlockType.REPORTER, opcode: '" + spec[2] + "', text: ";
			break;
		case "B":
			_blocks += "{blockType: BlockType.BOOLEAN, opcode: '" + spec[2] + "', text: ";
			break;
		}
		if(txtJpNew == "") {
			_blocks += "'" + txtEnNew + "', arguments: {\n";
		} else {
			_blocks += "[\n";
			_blocks += "    '" + txtEnNew + "',\n";
			_blocks += "    '" + txtJpNew + "',\n";
			_blocks += "][this._locale], arguments: {\n";
		}

		for(j = 0;j < argNum;j++) {
	//	ARG1: { type: ArgumentType.NUMBER, menu: 'led',	defaultValue:1,		type2:"B" },
			pos = args[j].indexOf(".");
			_blocks += "    ARG" + (j+1) + ": { type: ArgumentType." + ((types[j] == "s" || types[j].slice(0,1) == "b" || pos != -1) ? "STRING, ": "NUMBER, ")
					+ "type2:'" + types[j] + "', ";
			let init = spec[3+j];
			if(pos == -1) {
				if((types[j] == "s" || types[j].slice(0,1) == "b") || isNaN(Number(init))) init = "'"+init+"'";
				_blocks += "defaultValue:" + init +" },\n";
			} else {
				if(!(types[j] == "s" || types[j].slice(0,1) == "b") && ext.values.hasOwnProperty(init)) {
					init = ext.values[init];
				}
				_blocks += "defaultValue:'" + init +"', menu: '" + args[j].slice(pos+1) + "' },\n";
			}
		}
		if(obj.hasOwnProperty("hide")) {
			_blocks += "}, hideFromPalette:true},\n\n"
		} else {
			_blocks += "}},\n\n"
		}
	}

	let ids = [];
	for(const id in ext.menus)
		ids.push(id);

	ids.sort().forEach(id => {
		let values = ext.menus[id];

//	"noteJ1":["C2","D2","E2","F2","G2","A2","B2","C3","D3","E3","F3","G3","A3","B3",],

		menus += id + ": { acceptReporters: true, items: [";
		for(i=0;i<values.length;i++) {
			let en = values[i];
			if(!ext.values.hasOwnProperty(en)) {
				menus += "'" + en + "',";
			} else {
			//	{ text: 'ド4', value: 262 },
				if(i==0) menus += "\n";

				let val = en;
				val = ext.values[en];
				if(!ext.hasOwnProperty("translators") || !ext.translators.ja.hasOwnProperty(en)) {
					menus += "{ text: '" + en + "', value: '" + val + "' },\n";
				} else {
					menus += "{ text: ['" + en + "','" + ext.translators.ja[en] + "'][this._locale], value: '" + val + "' },\n";
				}
			}

		}
		menus += "]},\n\n";
	})

	let code = fs.readFileSync('Common/robot_pcmode.js.template', 'utf8');

	if(ext.boardType.split(":")[1] == "esp32") {
		code = code.replace("/*ESP32*", "")
					.replace("*ESP32*/", "");
	}

	code = code.replace("// DEFINE\n", define)
				.replace("// FLASHES\n", flashes)
				.replace("// CONSTRUCTOR\n", getProp(ext, "scratch3constructor"))
				.replace("// BLOCKS\n", getProp(ext, "scratch3blocks")+"'---',\n"+_blocks)
				.replace("// MENUS\n", menus+getProp(ext, "scratch3menus"))
				.replace("// FUNCS\n", funcs+getProp(ext, "scratch3funcs"));

	fs.writeFileSync('../scratch3/'+extNames[0]+'.js', code);

	code = code.replace( /\n\/\/\*\n/g, "\n/*_\n")		// \n//*\n
				.replace(/\n\/\/\*\/\n/g,"\n_*/\n")		// \n//*/\n
				.replace(/\n\/\*\n/g,  "\n//*\n")		// \n/*\n
				.replace(/\n\*\/\n/g,  "\n//*/\n")		// \n*/\n
				.replace("var extName = '" + extNames[0], "var extName = '" + extNames[0]+'0')
				.replace("color1:'#0FBD8C',color2:'#0DA57A',color3:'#0B8E69',", "color1:'#0F83BD',color2:'#0D73A6',color3:'#0B638F',")
				;

	//fs.writeFileSync(target+'/src/src.update.js', code);

	return true;
}

const jsonToCpp2 = function(target, ext)
{
	let code = fs.readFileSync('Common/robot_pcmode.ino.template', 'utf8');

	code = code.replace("// HEADER\n", getProp(ext, "header"))
				.replace("// SETUP\n", getProp(ext, "setup"))
				.replace("// LOOP\n", getProp(ext, "loop"));

	let argTbl = "";
	let work = "";
	for(let i=0; i<ext.blockSpecs.length; i++) {
		let spec = ext.blockSpecs[i];
		if(spec.length < 3){
			argTbl += "  {},\n";
			continue;
		}
		let obj = spec[spec.length-1];
		if(!obj.hasOwnProperty("remote")) {
			argTbl += "  {},\n";
			continue;
		}
		let offset=0;
		let j;
		for(j=0; ; j++) {
			offset = spec[1].indexOf('%', offset);
			if(offset<0) break;
			offset++;
		}

		let argNum = obj.remote.length + ((spec[0]=='R' || spec[0]=='B') ? -1 : 0);
		if(spec.length-4 != argNum || j != argNum) {
			let msg = "error in argument num of \""+spec[2]+"\": BlockSpec="+j+", init="+(spec.length-4)+", remote="+argNum;

			console.log(msg);
			return false;
		}

		argTbl += "  {";
		work += "case "+i.toString()+": ";

		let func = obj.func;
		for(j = 0; j<argNum; j++) {
			let getcmd = null;
			switch(obj.remote[j]) {
			case "B": getcmd = "getByte"; break;
			case "S": getcmd = "getShort"; break;
			case "L": getcmd = "getLong"; break;
			case "F": getcmd = "getFloat"; break;
			case "D": getcmd = "getDouble"; break;
			case "s": getcmd = "getString"; break;
			case "b": getcmd = "getBufLen"; break;
			case "b2": getcmd = "getBufLen2"; break;
			case "b3": getcmd = "getBufLen3"; break;
			}
			argTbl += "'"+obj.remote[j].slice(-1)+"',";
			func = func.replace(new RegExp("\\{"+j+"\\}", "g"), getcmd+"("+j.toString()+")");
		}

		switch(spec[0]) {
		case "w":
		//	case 1: remoconRobo_setRobot(getByte(0), getShort(1)); callOK(); break;
			work += func+"; callOK();";
			break;
		case "B":
		case "R":
		case "r":
		//	case 2: sendByte(pinMode(getByte(0), INPUT), digitalRead(getByte(0))); break;
			let setcmd = null;
			switch(obj.remote[obj.remote.length-1]) {
			case "B": setcmd = "sendByte"; break;
			case "S": setcmd = "sendShort"; break;
			case "L": setcmd = "sendLong"; break;
			case "F": setcmd = "sendFloat"; break;
			case "D": setcmd = "sendDouble"; break;
			case "s": setcmd = "sendString"; break;
			case "b": break;
			case "b2": break;
			case "b3": break;
			}
			work += (setcmd==null) ? func+";" : setcmd+"(("+func+"));";
			break;
		}
		argTbl += "},\n";
		work += " break;\n";
	}
	code = code.replace("// ARG_TYPES_TBL\n", argTbl);
	code = code.replace("// WORK\n", work);
	//code = fixTabs(code);

	fs.writeFileSync(target+'/src/src.ino', code);
	return true;
}

const copyBins = function(target, ext)
{
	const boards = ext.boardType.split(":");

	let src;
	let des;
	switch(boards[1]) {
	case "avr":
	default:
		src = target+"/src/build/src.ino.hex";
		des = target+"/src/src.ino.standard.hex";
		if(fs.existsSync(src))
			fs.copyFileSync(src, des);
		break;

	case "esp32":
		src = target+"/src/build/src.ino.bin";
		des = target+"/src/src.ino."+boards[2]+".bin";
		if(fs.existsSync(src))
			fs.copyFileSync(src, des);

		des = target+"/src/src.ino.bootloader.bin";
		src = des.replace("src/src","src/build/src");
		if(fs.existsSync(src))
			fs.copyFileSync(src, des);

		des = target+"/src/src.ino.partitions.bin";
		src = des.replace("src/src","src/build/src");
		if(fs.existsSync(src))
			fs.copyFileSync(src, des);
		break;
	}
	return true;
}

const _burnFW2 = function(target, ext, selectPort)
{
	const boards = ext.boardType.split(":");

	let hexFile;
	let cmd;
	let args;
	switch(boards[1]) {
	case "avr":
	default:
		hexFile = target+"/src/src.ino.standard.hex";
		if(!fs.existsSync(hexFile)){
			console.log("upgrade fail!");
			return false;
		}

		args = "-C"+toolsPath+"/tools/avr/etc/avrdude.conf -v -patmega328p -carduino -P"+selectPort+" -b115200 -D -V"
			+" -Uflash:w:"+hexFile+":i";
		cmd = toolsPath+"/tools/avr/bin/avrdude.exe";
		break;

	case "esp32":
		hexFile = target+"/src/src.ino."+boards[2]+".bin";
		if(!fs.existsSync(hexFile)){
			console.log("upgrade fail!");
			return false;
		}

		let baud = "921600";
		for(let i = 0; i < ext.prefs.length; i++) {
			let pref = ext.prefs[i];
			if(pref.indexOf("custom_UploadSpeed=")>=0) {
				pref = pref.substr("custom_UploadSpeed=".length);
				let pos = pref.indexOf("_");
				if(pos > 0) baud = pref.substr(pos+1);
				break;
			}
		}

		switch(boards[2]) {
		case "esp32":
		default:
			args = "--chip esp32 --port "+selectPort+" --baud "+baud+" --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect"
				+" 0x1000 " +toolsPath+"/tools/esp32/sdk/esp32/bin/bootloader_qio_80m.bin"
				+" 0x8000 " +target+"/src/src.ino.partitions.bin"
				+" 0xe000 " +toolsPath+"/tools/esp32/partitions/boot_app0.bin"
				+" 0x10000 "+hexFile;
			break;
		case "esp32c3":
			args = "--chip esp32c3 --port "+selectPort+" --baud "+baud+" --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect"
				+" 0x0 "    +toolsPath+"/tools/esp32/sdk/esp32c3/bin/bootloader_qio_80m.bin"
				+" 0x8000 " +target+"/src/src.ino.partitions.bin"
				+" 0xe000 " +toolsPath+"/tools/esp32/partitions/boot_app0.bin"
				+" 0x10000 "+hexFile;
			break;
		case "esp32s3":
			args = "--chip esp32s3 --port "+selectPort+" --baud "+baud+" --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect"
				+" 0x0 "    +target+"/src/src.ino.bootloader.bin"
				+" 0x8000 " +target+"/src/src.ino.partitions.bin"
				+" 0xe000 " +toolsPath+"/tools/esp32/partitions/boot_app0.bin"
				+" 0x10000 "+hexFile;
			break;
		}
		cmd = toolsPath+"/tools/esp32/esptool.exe";
		break;
	}

	cmd = cmd.replace('C:','/mnt/c')
	if(!fs.existsSync(cmd)){
		console.log("upgrade fail!");
		return false;
	}
	console.log(cmd + ' ' + args);
	return true;
}

const getRobotJson = function(target) {
  // " と " の間の\nを\\nに変換
  const lines = fs.readFileSync(target+'/robot.json', 'utf8').replace(/\r/g,'').split('\n');
  let i;
  let isMultiStr = false;
  for(i = 0; i < lines.length; i++) {
    const result = lines[i].split('//')[0].match(/"/g);
    if(result != null && (result.length & 1)) {
      isMultiStr = ! isMultiStr;
    }
    if(isMultiStr) {
      lines[i] += '\\n';
    } else {
      lines[i] += '\n';
    }
  }
  //console.log(lines.join(""));

  // robotJson パース
  eval('var tmp='+lines.join(""));

  if(!tmp.hasOwnProperty("prefs")) tmp.prefs = [];
  return tmp;
}

  // parse args
  const target = process.argv[2];
  let mode = 'params';
  if(process.argv.length >=4) {
    mode = process.argv[3];
  }

  const ext = getRobotJson(target);

  // 出力
  switch(mode) {
  case 'params':
    let param='--board ' + ext.boardType + ' --pref build.path=' + PWD + '/' + target + '/src/build';
    for(i = 0; i < ext.prefs.length; i++) {
      param += ' --pref ' + ext.prefs[i];
    }
    console.log(param);
    break;

  case 'jsonToCpp2':
    RET=jsonToCpp2(target, ext);
    break;

  case 'copyBins':
    RET=copyBins(target, ext);
    break;

  case 'jsonToJs':
    RET=jsonToJs(target, ext);
    break;

  case 'burnFW':
    RET=_burnFW2(target, ext, process.argv[4]);
    break;
  }
