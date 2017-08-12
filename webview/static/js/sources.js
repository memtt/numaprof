/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  GLOBALS  *********************/
var gblSourceEditor = null;
var selector = new NumaprofSelector();
var functions = {};
var template = "<li id='numaprof-func-list-entry'>\
					<a href='javascript:' data-toggle='popover' data-content='{{ longName }}' id='func-{{id}}'>\
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
	if (name.indexOf('separator') != 0)
	{
		selector.selectMetric(name);
		$("#numaprof-selected-metric").text(selector.getMetricName());
		updateFuncList();
		updateAnotations();
	}
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
	updateAnotations();
}

/*******************  FUNCTION  *********************/
function updateAnotations()
{
	gblSourceEditor.updateAnotations(false);
}

/*******************  FUNCTION  *********************/
function selectSearch(value)
{
	selector.query = value;
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
		if (selector.filter(functions[i]))
		{
			out.push({
				shortName: i,
				longName: i,
				file: functions[i].file,
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
	var firstFile = null;
	for (var i in out)
	{
		out[i].id = i;
		var rendered = Mustache.render(template, out[i]);
		$('#numaprof-func-list').append(rendered)
		$('#func-'+i).on('click',{value:out[i]},function(event) {
			console.log(event.data.value.file);
			gblSourceEditor.moveToFileFunction(event.data.value.file,event.data.value.longName);
		});;
		
		if (out[i].file != "??" && firstFile == null)
			firstFile = out[i].file;
	}
	$('[data-toggle="popover"]').popover({ trigger: "hover" });  
	
	if (gblSourceEditor.file == null && firstFile != null)
		gblSourceEditor.moveToFile(firstFile);
	else if (gblSourceEditor.file == null)
		gblSourceEditor.moveToFile(null);
}

/*******************  FUNCTION  *********************/
function loadFunctions()
{
	$.getJSON( "/api/sources/functions.json", function(data) {
		functions = data;
		for (var i in data)
			data[i].function = i;
		selector.setData(functions);
		updateFuncList();
	})
	.fail(function(data) {
		numaprofHelper.logError("Fail to load process summary");
	})
}

/*******************  FUNCTION  *********************/
$(function() {
	//exampleMenu();
	gblSourceEditor = new NumaprofSourceEditor('numaprof-source-editor',selector);
	//sourceEditor.moveToFile("/home/sebv/Projects/numaprof/src/core/ProcessTracker.cpp");
	setupSelectorList();
	Mustache.parse(template);   // optional, speeds up future uses
	loadFunctions();
});
