/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  GLOBALS  *********************/
var gblTotalTime = 1;
var gblCharts = {};

/*******************  FUNCTION  *********************/
function updateHeadMap(divId,data)
{
	//convert
	var fdata = [];
	for (var i in data)
		for (var j in data[i])
			if (i == -1)
				fdata.push({source:"Unpinned",dest:j,value:data[i][j]});
			else
				fdata.push({source:i,dest:j,value:data[i][j]});

	var heatmapColor = d3.scale.linear()
		.domain([d3.min(fdata, function(d) { return d.value; }), d3.max(fdata, function(d) { return d.value; })])
		.range(["#6363FF",  "#FF6364"]);
	
	var svg = d3.select(divId+' svg');
	
	var cells = svg.selectAll('rect')
		.data(fdata)
		.transition(500)
		.attr('fill', function(d) { return d.value == 0 ? "#CCCCCC" : heatmapColor(d.value); });
}

/*******************  FUNCTION  *********************/
function setupHeadMap(divId,data)
{
	var nodes = 0;
	for (var i in data)
		nodes++;
	
	var itemSize = 12,
		cellSize = itemSize - 1,
		margin = {top: 20, right: 20, bottom: 20, left: 110};
		
	var width = 60+2*itemSize*nodes - margin.right - margin.left,
		height = 20+itemSize*nodes - margin.top - margin.bottom;

	//convert
	var fdata = [];
	for (var i in data)
		for (var j in data[i])
			if (i == -1)
				fdata.push({source:"Unpinned",dest:j,value:data[i][j]});
			else
				fdata.push({source:i,dest:j,value:data[i][j]});

	var x_elements = d3.set(fdata.map(function( item ) { return item.dest; } )).values(),
		y_elements = d3.set(fdata.map(function( item ) { return item.source; } )).values();

	var xScale = d3.scale.ordinal()
		.domain(x_elements)
		.rangeBands([0, x_elements.length * itemSize]);

	var xAxis = d3.svg.axis()
		.scale(xScale)
		.tickFormat(function (d) {
			return d;
		})
		.orient("top");

	var yScale = d3.scale.ordinal()
		.domain(y_elements)
		.rangeBands([0, y_elements.length * itemSize]);

	var yAxis = d3.svg.axis()
		.scale(yScale)
		.tickFormat(function (d) {
			return d;
		})
		.orient("left");

	var colors = ["#6363FF", "#6373FF", "#63A3FF", "#63E3FF", "#63FFFB", "#63FFCB",
			"#63FF9B", "#63FF6B", "#7BFF63", "#BBFF63", "#DBFF63", "#FBFF63", 
			"#FFD363", "#FFB363", "#FF8363", "#FF7363", "#FF6364"];
	var heatmapColor = d3.scale.linear()
		.domain([d3.min(fdata, function(d) { return d.value; }), d3.max(fdata, function(d) { return d.value; })])
		.range(["#6363FF",  "#FF6364"]);
		//.range(colors);
		
	/* Initialize tooltip */
	var tip = d3.tip().attr('class', 'd3-tip').html(function(d) { return d.value; });

	var svg = d3.select(divId)
		.append("svg")
		.attr("width", width + margin.left + margin.right)
		.attr("height", height + margin.top + margin.bottom)
		.append("g")
		.attr("transform", "translate(" + margin.left + "," + margin.top + ")");
		
	svg.call(tip);

	var cells = svg.selectAll('rect')
		.data(fdata)
		.enter().append('g').append('rect')
		.attr('class', 'cell')
		.attr('width', cellSize)
		.attr('height', cellSize)
		.attr('y', function(d) { return yScale(d.source); })
		.attr('x', function(d) { return xScale(d.dest); })
		.attr('fill', function(d) { return d.value == 0 ? "#CCCCCC" : heatmapColor(d.value); })
		.on('mouseover',tip.show)
		.on('mouseout',tip.hige);

	svg.append("g")
		.attr("class", "y axis")
		.call(yAxis)
		.selectAll('text')
		.attr('font-weight', 'normal');

	svg.append("g")
		.attr("class", "x axis")
		.call(xAxis)
		.selectAll('text')
		.attr('font-weight', 'normal')
		.style("text-anchor", "start")
		.attr("dx", ".8em")
		.attr("dy", ".5em")
		.attr("transform", function (d) {
			return "rotate(-65)";
		});
}

/*******************  FUNCTION  *********************/
function genPieDataFirstTouch(datas)
{
	return [
		{
			"label": "Pinned",
			"value": numaprofHelper.valOrDefault(datas.firstTouch,0)
		},{
			"label": "Unpinned",
			"value": numaprofHelper.valOrDefault(datas.unpinnedFirstTouch,0)
		}
	];
}

/*******************  FUNCTION  *********************/
function genPieDataAccess(datas)
{
	return [
		{
			"label": "Local",
			"value": numaprofHelper.valOrDefault(datas.localAccess,0)
		},{
			"label": "Remote",
			"value": numaprofHelper.valOrDefault(datas.remoteAccess,0)
		},{
			"label": "Unpinned page",
			"value": numaprofHelper.valOrDefault(datas.unpinnedPageAccess,0)
		},{
			"label": "Unpinned thread",
			"value": numaprofHelper.valOrDefault(datas.unpinnedThreadAccess,0)
		},{
			"label": "Unpinned both",
			"value": numaprofHelper.valOrDefault(datas.unpinnedBothAccess,0)
		},{
			"label": "MCDRAM",
			"value": numaprofHelper.valOrDefault(datas.mcdramAccess,0)
		}
	];
}

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
			.showLegend(false)
			;

			d3.select(divName+" svg")
				.datum(data)
				.transition().duration(350)
				.call(chart);

		nv.utils.windowResize(function(){ chart.update(); });
		
		gblCharts[divName] = chart;
		
		return chart;
	});
}

/*******************  FUNCTION  *********************/
function updateChartData(divName,data)
{
	var chart = gblCharts[divName];
	d3.select(divName+' svg').datum(data).transition().duration(500).call(chart);
	nv.utils.windowResize(chart.update);
}

/*******************  FUNCTION  *********************/
function getMemPolicy(data)
{
	var mask = numaprofHelper.listToRange(data.mask);
	if (data.mask.lenth == 0)
		mask = "none";
	
	var text = data.mode+" on "+mask+" considered as "+data.type;
	
	var cl = "";
	if (data.type == "NO_BIND")
		cl = "warning";
	else if (data.type == "INTERLEAVE" || data.type == "BIND_MULTIPLE")
		cl = "info";
	else if (data.type == "DEFAULT" && data.mask.lenth > 1)
		cl = "warning";
	else
		cl = "success";
	
	return {
		text: text,
		class: cl
	};
}

/*******************  FUNCTION  *********************/
function setupMemPolicy(divId,data)
{
	infos = getMemPolicy(data);
	$(divId).text(infos.text);
	$(divId).removeClass("warning").removeClass("success").removeClass('info').addClass(infos.class);
}

/*******************  FUNCTION  *********************/
function getRelativeTime(x)
{
	return (100*(x / gblTotalTime)).toFixed(2)+"%";
}

/*******************  FUNCTION  *********************/
function setupPinningLog(divId,threadLog,memLog)
{
	//vars
	var log = {};
	
	//build common log
	for (var i in threadLog)
	{
		threadLog[i].type = 0;
		log[threadLog[i].at] = threadLog[i];
	}
	
	//build common log
	for (var i in memLog)
	{
		threadLog[i].type = 1;
		log[threadLog[i].at] = memLog[i];
	}
	
	//loop
	var ul = $(divId);
	for (var i in log)
	{
		if (log[i].type == 0)
		{
			ul.append("<li>At "+getRelativeTime(i)+", do thread on node "+log[i].numa+"</li>");
		} else if (log[i].type == 1) {
			var tmp = getMemPolicy(log[i]);
			ul.append("<li>At "+getRelativeTime(i)+", do memory binding "+tmp.text+"</li>");
		}
	}
}

/*******************  FUNCTION  *********************/
function moveToPage(data,pageNum,first)
{
	console.log(data);
	if (first)
	{
		setupPieChart("#threadFirstTouch",genPieDataFirstTouch(data[pageNum].stats));
		setupPieChart("#threadAccesses",genPieDataAccess(data[pageNum].stats));
		setupHeadMap("#accessMatrix",data[pageNum].accessMatrix);
	} else {
		updateChartData("#threadFirstTouch",genPieDataFirstTouch(data[pageNum].stats));
		updateChartData("#threadAccesses",genPieDataAccess(data[pageNum].stats));
		updateHeadMap("#accessMatrix",data[pageNum].accessMatrix);
	}
	var numaBinding = data[pageNum].numa;
	if (numaBinding == -1)
		$('#numaBinding').text("NOT BINDED").addClass("warning").removeClass("success");
	else
		$('#numaBinding').text(numaBinding).addClass("success").removeClass("warning");
	$('#lifetime').text(getRelativeTime(data[pageNum].clockStart)+" => "+getRelativeTime(data[pageNum].clockEnd));
	setupMemPolicy("#numaMemPolicy",data[pageNum].memPolicy);
	setupPinningLog("#pinningLog",data[pageNum].bindingLog,data[pageNum].memPolicyLog);
}

/*******************  FUNCTION  *********************/
function setupInitial(data)
{
	//keep it
	gblDetails = data;
	
	//compute pages
	var pages = data.length / 10;
	if (pages * 10 != data.length)
		pages++;
	
	//tmp
	pages = data.length;
	
	//setup paging
	$('#paging').bootpag({
		total: pages,
		page: 1,
		maxVisible: 10,
		leaps: true,
		firstLastUse: true,
		first: '←',
		last: '→',
		wrapClass: 'pagination',
		activeClass: 'active',
		disabledClass: 'disabled',
		nextClass: 'next',
		prevClass: 'prev',
		lastClass: 'last',
		firstClass: 'first'
	}).on("page", function(event, num){
		moveToPage(data,num,false);
	}); 
	
	moveToPage(data,1,true);
}

/*******************  FUNCTION  *********************/
function loadDetails()
{
	$.getJSON( "/api/details/threads.json", function(dataInfos) {
		if (dataInfos.duration != undefined)
			gblTotalTime = dataInfos.duration;
		$.getJSON( "/api/details/threads.json", function(data) {
			setupInitial(data);
		})
		.fail(function(data) {
			numaprofHelper.logError("Fail to load /api/details/threads.json");
		})
	})
	.fail(function(data) {
		numaprofHelper.logError("/api/details/threads.json");
	})
}

/*******************  FUNCTION  *********************/
$(function() {
	loadDetails();
});