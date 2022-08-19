'use strict'
const fs = require('fs');

fs.readFile('ATPlabels.txt','utf8', (err,data) => {
	if (err) {
		console.log("error brother");
		return;
	}

	const lines = data.split('</Enumerated>\n');

	const objArray = new Array();

	let label;

	lines.forEach( (line,index) => {
		if(line.split('<Label>').length > 1){
			label = line.split('<Label>')[1].split('</Label>')[0];
			console.log(label);
			
			let valueArray = new Array();
			const val = line.split('<Value>\n');

			val.forEach( (valTag,index1) => {
				if(index1){
					valueArray.push(valTag.split('<Code>').pop().split('</Code>')[0]);
					valueArray.push(valTag.split('<Label>').pop().split('</Label>')[0]);
				}
			});
			objArray.push({index:index,label:label,valueArray:valueArray});
		}
	});

	const str = objArray.map( obj => {
		let str1 = `${obj.index}\t${obj.label}`
		obj.valueArray.forEach( val => {
			str1 = str1.concat(`\t${val}`);
		});
		return str1;
	});

	fs.writeFile('ATP_LABELS.txt',str.join('\n'), function (err) {
		if (err) return console.log(err);
	});	
});


