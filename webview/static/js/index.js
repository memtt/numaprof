/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

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
				entry = $("<tr><th>"+node+"</th><td>"+data[node].cpus.join(',')+"</td>");
			$("#numaTopology").append(entry);
		}
	})
	.fail(function(data) {
		logError("Fail to load NUMA topology");
	})
}

/*******************  FUNCTION  *********************/
$(function() {
	loadInfos();
	loadNumaTopo();
});
