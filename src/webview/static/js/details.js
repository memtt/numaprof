/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  GLOBALS  *********************/
var gblTotalTime = 1;
var gblCharts = {};
var gblThreadPerPage = 2;

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
	
	var itemSize = 15,
		cellSize = itemSize - 1,
		margin = {top: 50, right: 20, bottom: 20, left: 110};
		
	var width = 120+2*itemSize*nodes - margin.right - margin.left,
		height = 100+itemSize*nodes - margin.top - margin.bottom;

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
	
	// text label for the x axis
	svg.append("text")             
		.attr("transform",
				"translate(" + (width/2) + " ," + 
							( -25) + ")")
		.style("text-anchor", "middle")
		.text("To");
		
	// text label for the y axis
	svg.append("text")
		.attr("transform", "rotate(-90)")
		.attr("y", 0 - margin.left)
		.attr("x",0 - (height / 2))
		.attr("dy", "1em")
		.style("text-anchor", "middle")
		.text("From"); 
}

/*******************  FUNCTION  *********************/
function genPieDataFirstTouch(datas)
{
	return [
		{
			"label": "Pinned",
			"value": numaprofHelper.valOrDefault(datas.firstTouch,0),
			"color": "rgb(44, 160, 44)",
		},{
			"label": "Unpinned",
			"value": numaprofHelper.valOrDefault(datas.unpinnedFirstTouch,0),
			"color": "rgb(255, 127, 14)",
		}
	];
}

/*******************  FUNCTION  *********************/
function genPieDataAccess(datas)
{
	return [
		{
			"label": "Local",
			"value": numaprofHelper.valOrDefault(datas.localAccess,0),
			"color": "rgb(44, 160, 44)"
		},{
			"label": "Remote",
			"value": numaprofHelper.valOrDefault(datas.remoteAccess,0),
			"color": "red"
		},{
			"label": "Unpinned page",
			"value": numaprofHelper.valOrDefault(datas.unpinnedPageAccess,0),
			"color": "rgb(255, 187, 120)",
		},{
			"label": "Unpinned thread",
			"value": numaprofHelper.valOrDefault(datas.unpinnedThreadAccess,0),
			"color": "rgb(255, 127, 14)",
		},{
			"label": "Unpinned both",
			"value": numaprofHelper.valOrDefault(datas.unpinnedBothAccess,0),
			"color": "rgb(31, 119, 180)",
		},{
			"label": "Local MCDRAM",
			"value": numaprofHelper.valOrDefault(datas.localMcdramAccess,0),
			"color": "#FF79DE"
		},{
			"label": "Remote MCDRAM",
			"value": numaprofHelper.valOrDefault(datas.remoteMcdramAccess,0),
			"color": "#54017a"
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
		memLog[i].type = 1;
		log[memLog[i].at] = memLog[i];
	}
	
	//loop
	var ul = $(divId);
	ul.html("");
	for (var i in log)
	{
		if (log[i].type == 0)
		{
			var n = $("<tr><td>At "+getRelativeTime(i)+", pin thread on node "+log[i].numa+"</td></tr>");
			if (log[i].numa == -1)
				n.addClass("warning");
			else
				n.addClass("success");
			ul.append(n);
		} else if (log[i].type == 1) {
			var tmp = getMemPolicy(log[i].policy);
			var n = $("<tr><td>At "+getRelativeTime(i)+", do memory binding "+tmp.text+"</td></tr>").addClass(tmp.class);
			ul.append(n);
		}
	}
}

/*******************  FUNCTION  *********************/
function displayThread(divId,data,threadId,first)
{
	if (first)
	{
		setupPieChart(divId + " .threadFirstTouch",genPieDataFirstTouch(data.stats));
		setupPieChart(divId + " .threadAccesses",genPieDataAccess(data.stats));
		setupHeadMap(divId + " .accessMatrix",data.accessMatrix);
		setupNumaDistanceStats(divId+ " .accessDistance",data.distanceCnt);
	} else {
		updateChartData(divId + " .threadFirstTouch",genPieDataFirstTouch(data.stats));
		updateChartData(divId + " .threadAccesses",genPieDataAccess(data.stats));
		updateHeadMap(divId + " .accessMatrix",data.accessMatrix);
		updateNumaDistanceStats(divId+ " .accessDistance",data.distanceCnt);
	}
	var numaBinding = data.numa;
	if (numaBinding == -1)
		$(divId+ ' .numaBinding').text("NOT BINDED").addClass("warning").removeClass("success");
	else
		$(divId + ' .numaBinding').text(numaBinding).addClass("success").removeClass("warning");
	$(divId + ' .cpuBinding').text(numaprofHelper.listToRange(data.binding));
	$(divId+ ' .lifetime').text(getRelativeTime(data.clockStart)+" → "+getRelativeTime(data.clockEnd));
	setupMemPolicy(divId + " .numaMemPolicy",data.memPolicy);
	setupPinningLog(divId + " .pinningLog",data.bindingLog,data.memPolicyLog);
	$(divId+" .threadId").text(threadId);
}

/*******************  FUNCTION  *********************/
function moveToPage(data,pageNum,first)
{
	pageNum -= 1;
	threads = gblThreadPerPage;
	
	if (first)
	{
		var template = $("#content").html();
		for (var i = 1 ; i < threads ; i++)
		{
			var n = $(template).attr('id',"infosThread"+i);
			$("#content").append(n);
		}
	}
	
	var threadId = [];
	for (var i = 0 ; i < threads ; i++)
	{
		threadId[i] = pageNum * 2 + i;
		if (threadId[i] >= data.length)
			threadId[i] = 0;
	}
	
	for (var i = 0 ; i < threads ; i++)
		displayThread("#infosThread"+i, data[threadId[i]],threadId[i],first);
}

/*******************  FUNCTION  *********************/
function setupInitial(data)
{
	//keep it
	gblDetails = data;
	
	//compute pages
	var pages = data.length / gblThreadPerPage;
	if (pages * gblThreadPerPage != data.length)
		pages++;
	
	var defaultPage = 1;
	if (data.length == 1)
		defaultPage = 0;
	
	//setup paging
	$('#paging').bootpag({
		total: pages,
		page: defaultPage,
		maxVisible: 10,
		leaps: false,
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
	
	$("#jump").on("change",function(cur) {
		var threadId = $("#jump").val();
		var page = 1+threadId / 2;
		if (page > pages)
			numaprofHelper.logError("Invalid thread number : "+threadId);
		$('#paging').bootpag({
			page:page
		});
		moveToPage(data,page,false);
	});
	
	moveToPage(data,1,true);
}

/*******************  FUNCTION  *********************/
function loadDetails()
{
	$.getJSON( "/api/index/infos.json", function(dataInfos) {
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
function updateNumaDistanceStats(divName,data)
{
	//reformat data
	var formattedData = [{
		key: "Numa distance",
		values: []
    }];
	
	for (var i in data)
		formattedData[0].values.push({x:i-1,y:data[i]});
	
	var chart = gblCharts[divName];
	d3.select(divName+' svg').datum(formattedData).transition().duration(500).call(chart);
	nv.utils.windowResize(chart.update);
}

/*******************  FUNCTION  *********************/
function setupNumaDistanceStats(divName,data)
{
	//reformat data
	var formattedData = [{
		key: "Numa distance",
		values: []
    }];
	
	for (var i in data)
		formattedData[0].values.push({x:i-1,y:data[i]});

	//plot
	nv.addGraph(function() {
		var chart = nv.models.multiBarChart()
			//.transitionDuration(350)
			//.reduceXTicks(true)   //If 'false', every single x-axis tick label will be rendered.
			//.rotateLabels(0)      //Angle to rotate x-axis labels.
			.showControls(false)   //Allow user to switch between 'Grouped' and 'Stacked' mode.
			//.groupSpacing(0.1)    //Distance between each group of bars.
		;

		// 	chart.xAxis
		// 		.tickFormat(d3.format(',f'));
		// 
		// 	chart.yAxis
		// 		.tickFormat(d3.format(',.1f'));

		d3.select(divName+' svg')
			.datum(formattedData)
			.call(chart);

		nv.utils.windowResize(chart.update);
		
		gblCharts[divName] = chart;

		return chart;
	});
}

/*******************  FUNCTION  *********************/
$(function() {
	loadDetails();
});
