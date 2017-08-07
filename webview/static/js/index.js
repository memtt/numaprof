/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  FUNCTION  *********************/
function setupPieChart(divName,data)
{
	//Donut chart example
	nv.addGraph(function() {
		var chart = nv.models.pieChart()
			.x(function(d) { return d.label })
			.y(function(d) { return d.value })
			.showLabels(true)     //Display pie labels
			.labelThreshold(.05)  //Configure the minimum slice size for labels to show up
			.labelType("percent") //Configure what type of data to show in the label. Can be "key", "value" or "percent"
			.donut(true)          //Turn on Donut mode. Makes pie chart look tasty!
			.donutRatio(0.35)     //Configure how big you want the donut hole size to be.
			;

			d3.select("#"+divName+" svg")
				.datum(data)
				.transition().duration(350)
				.call(chart);

		nv.utils.windowResize(function(){ chart.update(); });
		
		return chart;
	});
}

/*******************  FUNCTION  *********************/
function genPieDataFirstTouch(data)
{
	return [
		{
			"label": "Pinned",
			"value": data.metrics.firstTouch
		},{
			"label": "Unpinned",
			"value": data.metrics.unpinnedFirstTouch
		}
	];
}

/*******************  FUNCTION  *********************/
function genPieDataAccess(data)
{
	return [
		{
			"label": "Local",
			"value": data.metrics.localAccess
		},{
			"label": "Remote",
			"value": data.metrics.remoteAccess
		},{
			"label": "Unpinned page",
			"value": data.metrics.unpinnedPageAccess
		},{
			"label": "Unpinned thread",
			"value": data.metrics.unpinnedThreadAccess
		},{
			"label": "Unpinned both",
			"value": data.metrics.unpinnedBothAccess
		}
	];
}

/*******************  FUNCTION  *********************/
function loadInfos()
{
	$.getJSON( "/api/index/infos.json", function(data) {
		$("#summaryExe").text(data.exe);
		$("#summaryCommand").text(data.command);
		$("#summaryTool").text(data.tool);
		$("#summaryHostname").text(data.hostname);
		$("#summaryDate").text(data.date);
	})
	.fail(function(data) {
		logError("Fail to load infos");
	})
}

/*******************  FUNCTION  *********************/
function loadNumaTopo()
{
	$.getJSON( "/api/index/numa-topo.json", function(data) {
		for(var node in data)
		{
			console.log(node)
			var entry;
			if (data[node].isMcdram)
				entry = $("<tr><th>"+node+"</th><td>MCDRAM</td>");
			else
				entry = $("<tr><th>"+node+"</th><td>"+listToRange(data[node].cpus)+"</td>");
			$("#numaTopology").append(entry);
		}
	})
	.fail(function(data) {
		logError("Fail to load NUMA topology");
	})
}

/*******************  FUNCTION  *********************/
function loadProcessSummary()
{
	$.getJSON( "/api/index/process-summary.json", function(data) {
		setupPieChart("processFirstTouch",genPieDataFirstTouch(data));
		setupPieChart("processAccess",genPieDataAccess(data));
	})
	.fail(function(data) {
		logError("Fail to load process summary");
	})
}

/*******************  FUNCTION  *********************/
$(function() {
	loadInfos();
	loadNumaTopo();
	loadProcessSummary();
});
