/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  FUNCTION  *********************/
function setupBarChart(svgId,data)
{
	//nb
	var cnt = data[0].values.length
	
	//setup height
	svg = d3.select('#'+svgId+' svg').attr("height",100+cnt*30);
	
	console.log(data);
	
	nv.addGraph(function() {
		var chart = nv.models.multiBarHorizontalChart()
			.x(function(d) { return d.label })
			.y(function(d) { return d.value })
			.margin({top: 30, right: 30, bottom: 50, left: 65})
			//.showValues(true)           //Show bar value next to each bar.
			//.tooltips(true)             //Show tooltips on hover.
			//.transitionDuration(350)
			.showControls(false)
			.stacked(true);        //Allow user to switch between "Grouped" and "Stacked" mode.

		chart.yAxis
			.tickFormat(d3.format(',.2f'));

		d3.select('#'+svgId+' svg')
			.datum(data)
			.call(chart);

		nv.utils.windowResize(chart.update);
		
		chart.multibar.dispatch.on("elementDblClick", function(e) {
			//remove
			for (var i in data)
				data[i].values[e.index].value = 0;
			//update
			d3.select('#'+svgId+' svg').datum(data).transition().duration(500).call(chart);
			nv.utils.windowResize(chart.update);
		});

		return chart;
	});
}

/*******************  FUNCTION  *********************/
function genFirstTouchData(data)
{
	//base
	out = [{
		"key": "Pinned",
		"values": [],
		"color": "rgb(44, 160, 44)",
	},{
		"key": "Unpinned",
		"values": [],
		"color": "rgb(255, 127, 14)",
	}];
	
	//for all threads
	for (var i in data)
	{
		var stats = data[i].stats;
		out[0].values.push({label:"Thread "+i,value:numaprofHelper.valOrDefault(stats.firstTouch,0)});
		out[1].values.push({label:"Thread "+i,value:numaprofHelper.valOrDefault(stats.unpinnedFirstTouch,0)});
	}
	
	return out;
}

/*******************  FUNCTION  *********************/
function genFirstAccess(data)
{
	//base
	out = [{
		"key": "unpinnedPageAccess",
		"values": [],
		"color": "rgb(255, 187, 120)",
	},{
		"key": "unpinnedThreadAccess",
		"values": [],
		"color": "rgb(255, 127, 14)",
	},{
		"key": "unpinnedBothAccess",
		"values": [],
		"color": "rgb(31, 119, 180)",
	},{
		"key": "localAccess",
		"values": [],
		"color": "rgb(44, 160, 44)"
	},{
		"key": "remoteAccess",
		"values": [],
		"color": "red"
	},{
		"key": "localMcdramAccess",
		"values": [],
		"color": "#FF79DE"
	},{
		"key": "remoteMcdramAccess",
		"values": [],
		"color": "#54017a"
	}];
	
	//for all threads
	for (var i in data)
	{
		var stats = data[i].stats;
		out[0].values.push({label:"Thread "+i,value:numaprofHelper.valOrDefault(stats.unpinnedPageAccess,0)});
		out[1].values.push({label:"Thread "+i,value:numaprofHelper.valOrDefault(stats.unpinnedThreadAccess,0)});
		out[2].values.push({label:"Thread "+i,value:numaprofHelper.valOrDefault(stats.unpinnedBothAccess,0)});
		out[3].values.push({label:"Thread "+i,value:numaprofHelper.valOrDefault(stats.localAccess,0)});
		out[4].values.push({label:"Thread "+i,value:numaprofHelper.valOrDefault(stats.remoteAccess,0)});
		out[5].values.push({label:"Thread "+i,value:numaprofHelper.valOrDefault(stats.localMcdramAccess,0)});
		out[6].values.push({label:"Thread "+i,value:numaprofHelper.valOrDefault(stats.remoteMcdramAccess,0)});
	}
	
	return out;
}

/*******************  FUNCTION  *********************/
function printThreadBinding(data)
{
	var nodes = {};
	for (var i in data)
	{
		if (nodes[data[i].numa] == undefined)
			nodes[data[i].numa] = [];
		nodes[data[i].numa].push(i);
	}
	
	for (var i in nodes)
	{
		var entry;
		if (i == -1)
			entry = $("<tr><th>Not binded</th><td>"+numaprofHelper.listToRange(nodes[i])+"</td>");
		else
			entry = $("<tr><th>"+i+"</th><td>"+numaprofHelper.listToRange(nodes[i])+"</td>");
		$("#numaTopology").append(entry);
	}
}

/*******************  FUNCTION  *********************/
function loadThreadInfos()
{
	$.getJSON( "/api/threads/infos.json", function(data) {
		setupBarChart("threadsFirstTouch",genFirstTouchData(data));
		setupBarChart("threadsAccess",genFirstAccess(data));
		printThreadBinding(data);
	})
	.fail(function(data) {
		numaprofHelper.logError("Fail to load process summary");
	})
}

/*******************  FUNCTION  *********************/
$(function() {
	loadThreadInfos();
});
