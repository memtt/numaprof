/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  GLOBALS  *********************/
var selector = new NumaprofSelector();
var functions = {};
var search = "";
var template = "<li id='numaprof-func-list-entry'>\
					<a href='javascript:' data-toggle='popover' data-content='{{ longName }}'>\
						<span class='size'>\
							{{fvalue}}\
							<div class='progress-fg' style='width: {{ ratio }}%;'></div>\
							<div class='progress-bg'></div>\
						</span>\
						<span class='func' ng-hide='compact'>{{ shortName }}</span>\
					</a>\
				</li>";


/*******************  FUNCTION  *********************/
function selectMetric(name)
{
	console.log("Select metric: "+ name);
	selector.selectMetric(name);
	$("#numaprof-selected-metric").text(selector.getMetricName());
	updateFuncList();
}

/*******************  FUNCTION  *********************/
function selectRatio()
{
	if (selector.ratio)
	{
		selector.ratio = false;
		$("#numaprof-ratio-select").removeClass("active");
	} else {
		selector.ratio = true;
		$("#numaprof-ratio-select").addClass("active");
	}
	updateFuncList();
}

/*******************  FUNCTION  *********************/
function selectSearch(value)
{
	search = value;
	updateFuncList();
}

/*******************  FUNCTION  *********************/
function setupSelectorList()
{
	var metrics = selector.getMetrics();
	console.log(metrics);
	for (var i in metrics)
	{
		entry = $("<li><a>"+metrics[i].name+"</a></li>").on("click",{ sel: i },function(event) {
			selectMetric(event.data.sel);
		});
		$("#numaprof-selector-list").append(entry);
	}
	$("#numaprof-ratio-select").on("click",function() {selectRatio();});
	$("#numaprof-input-search").on("keypress",function() {selectSearch($("#numaprof-input-search").val());});
	selectRatio();
	selectMetric(selector.metric);
}

/*******************  FUNCTION  *********************/
function updateFuncList()
{
	//build list
	var out = [];
	for (var i in functions)
	{
		if (search == "" || i.indexOf(search) != -1)
		{
			out.push({
				shortName: i,
				longName: i,
				value: selector.getValue(functions[i]),
				fvalue: selector.getFormattedValue(functions[i]),
				ratio: selector.getValueRatio(functions[i])
			});
		}
	}
	
	//sort
	out.sort(function(a,b) {return b.value-a.value;});
	
	//select N first
	out = out.slice(0,10);
	
	//clear
	$('#numaprof-func-list').html("")
	
	//put
	for (var i in out)
	{
		var rendered = Mustache.render(template, out[i]);
		$('#numaprof-func-list').append(rendered);
	}
	$('[data-toggle="popover"]').popover({ trigger: "hover" });  
}

/*******************  FUNCTION  *********************/
function loadFunctions()
{
	$.getJSON( "/api/sources/functions.json", function(data) {
		functions = data;
		selector.setData(functions);
		updateFuncList();
	})
	.fail(function(data) {
		numaprofHelper.logError("Fail to load process summary");
	})
}

/*******************  FUNCTION  *********************/
function exampleMenu()
{
	lst = [
		{value:"60%", ratio:"60",shortName:"shortName",longName:"lllllooooooonnnnnngggg_naaaammme"},
		{value:"60%", ratio:"60",shortName:"shortName2",longName:"lllllooooooonnnnnnggggnaaaammme"},
		{value:"60%", ratio:"60",shortName:"shortName3",longName:"aaa"},
	];
	
	var template = "<li id='numaprof-func-list-entry'>\
					<a href='javascript:' data-toggle='popover' data-content='{{ longName }}'>\
						<span class='size'>\
							{{value}}\
							<div class='progress-fg' style='width: {{ ratio }}%;'></div>\
							<div class='progress-bg'></div>\
						</span>\
						<span class='func' ng-hide='compact'>{{ shortName }}</span>\
					</a>\
				</li>";
	Mustache.parse(template);   // optional, speeds up future uses
	$('#numaprof-func-list-entry').html("")
	
	for (var i in lst)
	{
		var rendered = Mustache.render(template, lst[i]);
		$('#numaprof-func-list').append(rendered);
	}
	$('[data-toggle="popover"]').popover({ trigger: "hover" });  
}

/*******************  FUNCTION  *********************/
$(function() {
	exampleMenu();
	var sourceEditor = new MaltSourceEditor('numaprof-source-editor',undefined);
	sourceEditor.moveToFile("/home/sebv/Projects/numaprof/src/core/ProcessTracker.cpp");
	setupSelectorList();
	Mustache.parse(template);   // optional, speeds up future uses
	loadFunctions();
});
