'use strict'
const fs = require('fs');

fs.readFile('ATPsynthesis.txt','utf8', (err,data) => {
	if (err) {
		console.log("error brother");
		return;
	}
	fs.readFile('ATP_LABELS.txt','utf8', (err,labels) => {
		if (err) {
			console.log("error brother");
			return;
		}
		fs.readFile('NumToHeaderString_ATP.txt','utf8', (err,actualID) => {
			if (err) {
				console.log("error brother");
				return;
			}

			const actualMap = new Map();
			actualID.split('\n').forEach( actualLine => {
				actualMap.set(actualLine.split('\t')[0],actualLine.split('\t')[1]);
			});

			const enumeratedMap = new Map();
			labels.split('\n').forEach( labelLine => {
				enumeratedMap.set(labelLine.split('\t')[1],labelLine.split('\t')[0]);
			});

			const typeMap = new Map([['Enumerated',0],['Hexadecimal',1],['Decimal',2],['Binary',3],['Date',4],['Time',5]]);

			const lines = data.split('\n');

			let label;
			let unsignedInteger;
			let bitPosition;
			let bitCount;
			let quantum;
			let offset;
			let displayType;
			let enumeratedLabel = -1;
			let decimalCount = -1;
			let unit = -1;

			const objArray = new Array();

			lines.forEach( line => {

				if(line.includes('<Field>')){
					enumeratedLabel = -1;
					decimalCount = -1;
					unit = -1;
				}
				else if(line.includes('Unsigned_integer')){
					unsignedInteger = 0;
				}
				else if(line.includes('Signed_integer')){
					unsignedInteger = 1;
				}
				else if(line.includes('<Label>')){
					label = line.split('<Label>').pop().split('</Label>')[0];
				}
				else if(line.includes('<First_bit_position>')){
					bitPosition = +line.split('<First_bit_position>').pop().split('</First_bit_position>')[0];
				}
				else if(line.includes('<Bit_count>')){
					bitCount = +line.split('<Bit_count>').pop().split('</Bit_count>')[0];
				}
				else if(line.includes('<Quantum>')){
					quantum = line.split('<Quantum>').pop().split('</Quantum>')[0];
				}
				else if(line.includes('<Offset>')){
					offset = line.split('<Offset>').pop().split('</Offset>')[0];
				}
				else if(line.includes('<Type>')){
					displayType = line.split('<Type>').pop().split('</Type>')[0];
				}
				else if(line.includes('<Enumerated_label>')){
					enumeratedLabel = line.split('<Enumerated_label>').pop().split('</Enumerated_label>')[0];
				}
				else if(line.includes('<Decimal_count>')){
					decimalCount = line.split('<Decimal_count>').pop().split('</Decimal_count>')[0];
				}
				else if(line.includes('<Unit>')){
					unit = line.split('<Unit>').pop().split('</Unit>')[0];
				}
				else if(line.includes('</Field>')){
					objArray.push({label:label,unsignedInteger:unsignedInteger,bitPosition:bitPosition,bitCount:bitCount,quantum:quantum,offset:offset,displayType:displayType,enumeratedLabel:enumeratedLabel,decimalCount:decimalCount,unit:unit});
				}
			});

			objArray.sort( (a,b) => {
				if ( a.bitPosition < b.bitPosition ){
					return -1;
				}
				if ( a.bitPosition > b.bitPosition ){
					return 1;
				}
				return 0;
			});

			const str = objArray.map( obj => {
				return `${actualMap.get(obj.label)-1}\t${obj.label}\t${obj.unsignedInteger}\t${obj.bitPosition}\t${obj.bitCount}\t${Math.floor(obj.bitPosition/8)}\t${Math.floor((obj.bitPosition+obj.bitCount-1)/8)}\t${obj.quantum}\t${obj.offset}\t${typeMap.get(obj.displayType)}\t${obj.enumeratedLabel == -1 ? -1 : enumeratedMap.get(obj.enumeratedLabel)}\t${obj.decimalCount}\t${obj.unit}`;
			});

			fs.writeFile('ATP_PARAMS.txt',str.join('\n'), function (err) {
				if (err) return console.log(err);
			});	
		});
	});
});


