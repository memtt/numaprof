/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  GLOBALS  *********************/
var gblSourceEditor = null;
var pageSelector = {pages:1,page:1,perPage:10};
var selector = new NumaprofSelector();
var functions = {};
var preparedFunctions = [];
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
		filterAndOrderFuncList();
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
	filterAndOrderFuncList();
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
	filterAndOrderFuncList();
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
	
	//setup paging
	$('#paging').bootpag({
		total: pageSelector.pages,
		page: pageSelector.page,
		maxVisible: 6,
		leaps: false,
		firstLastUse: false,
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
		pageSelector.page = num;
		updateFuncList();
	}); 
}

/*******************  FUNCTION  *********************/
function filterAndOrderFuncList()
{
	//filter
	out = [];
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
	
	//export
	preparedFunctions = out;
}

/*******************  FUNCTION  *********************/
function updateFuncList()
{
	//build list
	var out = preparedFunctions;
	
	//select N first
	start = pageSelector.perPage*(pageSelector.page-1);
	out = out.slice(start,start+pageSelector.perPage);
	
	//clear
	$('#numaprof-func-list').html("")
	
	//put
	var firstFile = null;
	var firstFunc = null;
	for (var i in out)
	{
		out[i].id = i;
		var rendered = Mustache.render(template, out[i]);
		$('#numaprof-func-list').append(rendered)
		$('#func-'+i).on('click',{value:out[i]},function(event) {
			console.log(event.data.value.file);
			gblSourceEditor.moveToFileFunction(event.data.value.file,event.data.value.longName);
		});;
		
		if (out[i].file != "??" && out[i].longName && firstFile == null)
			firstFile = out[i].file;
	}
	$('[data-toggle="popover"]').popover({ trigger: "hover" });  
	
	if (gblSourceEditor.file == null && firstFile != null)
		if (firstFunc != null)
			gblSourceEditor.moveToFileFunction(firstFile,firstFunc);
		//else
			//gblSourceEditor.moveToFile("??");
	else if (gblSourceEditor.file == null)
		gblSourceEditor.moveToFile(null);
}

/*******************  FUNCTION  *********************/
function updatePaging()
{
	len = Object.keys(functions).length;
	console.log("====================");
	console.log(len);
	console.log(len / pageSelector.perPage);
	$('#paging').bootpag({
		page:1,
		total:Math.ceil(len / pageSelector.perPage)
	});
}

/*******************  FUNCTION  *********************/
function loadFunctions()
{
	var select = "sources";
	if (gblIsAsm)
		select = "asm";
	
	$.getJSON( "/api/"+select+"/functions.json", function(data) {
		console.log(data);
		functions = data;
		for (var i in data)
			data[i].function = i;
		selector.setData(functions);
		updatePaging();
		filterAndOrderFuncList();
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
