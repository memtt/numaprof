/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  FUNCTION  *********************/
function setupBarChart(svgId,data)
{
	//nb
	var cnt = data[0].values.length
	
	//setup height
	d3.select('#'+svgId+' svg').attr("height",cnt*30);
	
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

		return chart;
	});
}

/*******************  FUNCTION  *********************/
function genFirstTouchData(data)
{
	//base
	out = [{
		"key": "Pinned",
		"values": []
	},{
		"key": "Unpinned",
		"values": []
	}];
	
	//for all threads
	for (var i in data)
	{
		var stats = data[i].stats;
		out[0].values.push({label:"Thread "+i,value:valOfDefault(stats.firstTouch,0)});
		out[1].values.push({label:"Thread "+i,value:valOfDefault(stats.unpinnedFirstTouch,0)});
	}
	
	return out;
}

/*******************  FUNCTION  *********************/
function genFirstAccess(data)
{
	//base
	out = [{
		"key": "unpinnedPageAccess",
		"values": []
	},{
		"key": "unpinnedThreadAccess",
		"values": []
	},{
		"key": "unpinnedBothAccess",
		"values": []
	},{
		"key": "localAccess",
		"values": []
	},{
		"key": "remoteAccess",
		"values": []
	},{
		"key": "mcdramAccess",
		"values": []
	}];
	
	//for all threads
	for (var i in data)
	{
		var stats = data[i].stats;
		out[0].values.push({label:"Thread "+i,value:valOfDefault(stats.unpinnedPageAccess,0)});
		out[1].values.push({label:"Thread "+i,value:valOfDefault(stats.unpinnedThreadAccess,0)});
		out[2].values.push({label:"Thread "+i,value:valOfDefault(stats.unpinnedBothAccess,0)});
		out[3].values.push({label:"Thread "+i,value:valOfDefault(stats.localAccess,0)});
		out[4].values.push({label:"Thread "+i,value:valOfDefault(stats.remoteAccess,0)});
		out[5].values.push({label:"Thread "+i,value:valOfDefault(stats.mcdramAccess,0)});
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
			entry = $("<tr><th>Not binded</th><td>"+listToRange(nodes[i])+"</td>");
		else
			entry = $("<tr><th>"+i+"</th><td>"+listToRange(nodes[i])+"</td>");
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
		logError("Fail to load process summary");
	})
}

/*******************  FUNCTION  *********************/
$(function() {
	loadThreadInfos();
});
