'use strict'
const fs = require('fs');

fs.readFile('ATO_PARAMS.txt','utf8', (err,data) => {
	if (err) {
		console.log("error brother");
		return;
	}

	const lines = data.split('\n');

	const arr = lines.map( line => {
		return line.split('\t')[8];
	});

	const set1 = new Set(arr);

	fs.writeFile('ATO_DISPLAYTYPES.txt',[...set1].join('\n'), function (err) {
		if (err) return console.log(err);
	});	
});


