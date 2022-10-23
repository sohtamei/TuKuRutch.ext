(function(ext) {
	var device = null;
	var checkDevName = false;
	var devName = "";

	var CMD_RESET = 0x7F;

	ext.resetAll = function(){
		device.send([0xff, 0x55, 1, CMD_RESET]);
	};
	ext.runArduino = function(){
		responseValue();
	};

	//### CUSTOMIZED ###
	var remoteKey = 0;
	var remoteX = 0;
	var remoteY = 0;
	
	var CMD_CHECKREMOTEKEY = 0x80;

	ext.checkRemoteKey = function() {
		sendPackage(CMD_CHECKREMOTEKEY);
	}
	ext.checkRemoteKey2 = function() {
		sendPackage(CMD_CHECKREMOTEKEY);
	}
	ext.isRemoteKey = function(code){
		responseValue2(0,remoteKey==code);
	}
	ext.isARemoteKey = function(code){
		responseValue2(0,remoteKey==code);
	}
	ext.getRemoteX = function(){
		responseValue2(0,remoteX);
	}
	ext.getRemoteY = function(){
		responseValue2(0,remoteY);
	}

	//### CUSTOMIZED END ###

	function sendPackage(){
		checkDevName = false;

		var bytes = [0xff, 0x55, 0];
		for(var i=0;i<arguments.length;++i)
			bytes.push(arguments[i]);
		bytes[2] = bytes.length - 3;
		device.send(bytes);
	}

	var rtype = {
		BYTE	: 1,
		SHORT	: 2,
		LONG	: 3,
		FLOAT	: 4,
		DOUBLE	: 5,
		STRING	: 6,
		BYTES	: 7,
	};

	var _rxBuf = [];
	var _packetLen = 3;
	function processData(bytes) {
		if(_rxBuf.length == 0 && bytes.length >= 2 && bytes[0] == 0xff && (bytes[1] == 0x55||bytes[1] == 0x54))
			checkDevName = false;

		if(checkDevName) {
			for(var index = 0; index < bytes.length; index++) {
				var c = bytes[index];
				if(c == 0x0d) {
					updateDevName(devName);
					checkDevName = false;
				} else {
					devName += String.fromCharCode(c);
				}
			}
			return;
		}

		for(var index = 0; index < bytes.length; index++){
			var c = bytes[index];	// 0xFF,0x55,len,type,
			_rxBuf.push(c);
			switch(_rxBuf.length) {
			case 1:
				_packetLen = 4;
				if(c != 0xff) 
					_rxBuf = [];
				break;
			case 2:
				if(c != 0x55 && c != 0x54)
					_rxBuf = [];
				break;
			case 3:
				if(_rxBuf[1] == 0x55)
					_packetLen = 3+c;
				break;
			case 4:
				if(_rxBuf[1] == 0x54)
					_packetLen = 4+_rxBuf[2]+(c<<8);
				break;
			}

			if(_rxBuf.length >= _packetLen) {
				if(_packetLen == 3) {
					responseValue();
				} else if(_rxBuf[1] == 0x54) {
					var value = readBytes(_rxBuf, 4, _packetLen-4);	break;
					responseValue(0,value);
				} else {
					var value = 0;
					switch(_rxBuf[3]) {
					case rtype.BYTE:	value = _rxBuf[4];				break;
					case rtype.SHORT:	value = readInt(_rxBuf, 4, 2);	break;
					case rtype.LONG:	value = readInt(_rxBuf, 4, 4);	break;
					case rtype.FLOAT:	value = readFloat(_rxBuf, 4);	break;
					case rtype.DOUBLE:	value = readDouble(_rxBuf, 4);	break;
					case rtype.STRING:	value = readString(_rxBuf, 4, _rxBuf[2]-1);	break;
					case rtype.BYTES:	value = readBytes(_rxBuf, 5, _rxBuf[2]-2);	break;

					//### CUSTOMIZED ###
					case CMD_CHECKREMOTEKEY:
						value = _rxBuf[4];
						remoteKey = value;
						remoteX = readInt(_rxBuf, 5, 2);
						remoteY = readInt(_rxBuf, 7, 2);
						break;
					}
					responseValue(0,value);
				}
				_rxBuf = [];
			}
		}
	}
	function readInt(arr,position,count){
		var result = 0;
		for(var i=0; i<count; ++i){
			result |= arr[position+i] << (i << 3);
		}
		if(arr[position+i-1] & 0x80) {
			result -= 1 << (i << 3);
		}
		return result;
	}
	function readFloat(arr,pos){
		var f= [arr[pos+0],arr[pos+1],arr[pos+2],arr[pos+3]];
		return parseFloat(f);
	}
	function readDouble(arr,position){
		var f= [arr[pos+0],arr[pos+1],arr[pos+2],arr[pos+3],arr[pos+4],arr[pos+5],arr[pos+6],arr[pos+7]];
		return parseFloat(f);
	}
	function readString(arr,position,len){
		var value = "";
		for(var ii=0;ii<len;ii++){
			value += String.fromCharCode(arr[ii+position]);
		}
		return value;
	}
	function readBytes(arr,position,len){
	}

	// Extension API interactions

	var resetReq = 0;
	ext._deviceConnected = function(dev, _checkDevName) {
		device = dev;
		checkDevName = true;
		resetReq = !_checkDevName;
		devName = "";
		device.set_receive_handler(processData);
		if(resetReq) {
			var bytes = [0,0xFF,0x55,0x01,0xFE];  // firmware name
			device.send(bytes);
		}
	};

	ext._deviceRemoved = function(dev) {
		if(device != dev) return;
		if(resetReq) {
			var bytes = [0xFF,0x55,0x01,0xFF];  // reset
			device.send(bytes);
		}
		device = null;
	};
	ext._getStatus = function() {
		if(!device) return {status: 1, msg: 'Disconnected'};
		return {status: 2, msg: 'Connected'};
	}
	var descriptor = {};
	ScratchExtensions.register('RemoconRobo', descriptor, ext, {type: 'serial'});
})({});
