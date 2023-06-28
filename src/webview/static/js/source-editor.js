/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

var tpl = "<table class='table table-borders source-popover-details'>\
			<tbody>\
				<tr>\
					<th>Pinned first touch</th>\
					<td>{{firstTouch}}</td>\
					<td><div width='20px' height='20px' style='background-color:rgb(44, 160, 44); width:20px; height:20px;'> </div></td>\
				</tr>\
				<tr>\
					<th>Unpinned first touch</th>\
					<td>{{unpinnedFirstTouch}}</td>\
					<td><div width='20px' height='20px' style='background-color:rgb(255, 127, 14); width:20px; height:20px;'> </div></td>\
				</tr>\
				<tr>\
					<th>Local</th>\
					<td>{{localAccess}}</td>\
					<td><div width='20px' height='20px' style='background-color:rgb(44, 160, 44); width:20px; height:20px;'> </div></td>\
				</tr>\
				<tr>\
					<th>Remote</th>\
					<td>{{remoteAccess}}</td>\
					<td><div width='20px' height='20px' style='background-color:red; width:20px; height:20px;'> </div></td>\
				</tr>\
				<tr>\
					<th>Unpinned page</th>\
					<td>{{unpinnedPageAccess}}</td>\
					<td><div width='20px' height='20px' style='background-color:rgb(255, 187, 120); width:20px; height:20px;'> </div></td>\
				</tr>\
				<tr>\
					<th>Unpinned thread</th>\
					<td>{{unpinnedThreadAccess}}</td>\
					<td><div width='20px' height='20px' style='background-color:rgb(255, 127, 14); width:20px; height:20px;'> </div></td>\
				</tr>\
				<tr>\
					<th>Unpinned both</th>\
					<td>{{unpinnedBothAccess}}</td>\
					<td><div width='20px' height='20px' style='background-color:rgb(31, 119, 180); width:20px; height:20px;'> </div></td>\
				</tr>\
				<tr>\
					<th>Local MCDRAM</th>\
					<td>{{localMcdramAccess}}</td>\
					<td><div width='20px' height='20px' style='background-color:#FF79DE; width:20px; height:20px;'> </div></td>\
				</tr>\
				<tr>\
					<th>Remote MCDRAM</th>\
					<td>{{remoteMcdramAccess}}</td>\
					<td><div width='20px' height='20px' style='background-color:#54017a; width:20px; height:20px;'> </div></td>\
				</tr>\
				<tr>\
					<th>Non allocated</th>\
					<td>{{nonAlloc}}</td>\
					<td></td>\
				</tr>\
			</tbody>\
		</table>\
";

/********************************************************************/
/**
 * Provide a source code editor to annotate sources with profiled values.
 * @param containerId Provide the ID of the div in which to setup the code editor.
 * @param selector Define a metric selector to extract call sites infos for annotations
**/
function NumaprofSourceEditor(containerId,selector)
{
	//errors
	//maltHelper.assert(selector != undefined && selector instanceof MaltSelector);
	
	//setup container
	this.containerId = containerId;
	this.container = document.getElementById(containerId);
	this.syntaxHighlighterEle = null;
	this.popoverTemplate = tpl;// $("#source-popover-template").html();
	Mustache.parse(this.popoverTemplate);
	//check and link
	//maltHelper.assert(this.container != undefined);
	this.container.malt = this;
	
	//defaults
	this.file = null;
	this.data = null;
	this.postMove = null;
	this.selector = selector;

	// To allow table elements
	//https://getbootstrap.com/docs/3.4/javascript/#js-sanitizer
	//var popOverWhiteList = $.fn.tooltip.Constructor.DEFAULTS.whiteList
	//popOverWhiteList.table = []
	//popOverWhiteList.tr = []
	//popOverWhiteList.td = []

}

/********************************************************************/
//To be override by user to capture events
NumaprofSourceEditor.prototype.onClick = function(infos)
{
	alert(JSON.stringify(infos,null,'\t'));
}

/********************************************************************/
NumaprofSourceEditor.prototype.doPostMove = function()
{
	if (this.postMove == null)
		return;
	if (this.postMove.type == 'line' && this.postMove.line != -1)
	{
		// this.editor.setCursor(this.postMove.line-1);
		Prism.plugins.codeAnnotator.scrollToAndHighlight(
			this.syntaxHighlighterEle, this.postMove.line);
	} else if (this.postMove.type == 'func') {
		var line = this.findLargestAnnot(this.file,this.postMove.func);
		if (line != -1)
			Prism.plugins.codeAnnotator.scrollToAndHighlight(
				this.syntaxHighlighterEle, line);

	}
}

/********************************************************************/
NumaprofSourceEditor.prototype.getLanguage = function(filename)
{
	if (gblIsAsm)
		return "nasm";
	
	//set mode
	var ext = (/[.]/.exec(filename)) ? /[^.]+$/.exec(filename) : undefined;
	if (ext != undefined)
		ext = (ext+"").toLowerCase();

	switch(ext)
	{
		case 'f':
		case 'f90':
		case 'f77':
			return "Fortran";
		case 'c':
		case 'h':
		case 'cxx':
		case 'cpp':
		case 'hxx':
		case 'h++':
		case 'hpp':
		case 'ainsic':
			return "C++";
		case 'py':
			return "Python";
		default:
			return "C++";
	}
}

/********************************************************************/
NumaprofSourceEditor.prototype.getLanguageClassForHighlighter = function(name) {
	name = name.toLowerCase();
	switch(name) 
	{
		case 'fortan':
			return 'language-fortran';
		case 'c':
		case 'c++':
		case 'C++':
			return 'language-cpp';
		case 'python':
			return 'language-python';
		case 'nasm':
			return 'language-nasm';
		default:
			return '';
	}
}

/********************************************************************/
NumaprofSourceEditor.prototype.loadSourceFile = function(file,func,success,fail)
{
	var select = "sources";
	if (gblIsAsm)
		select = "asm";
	
	//handle relative paths
	file = this.fixRelativePath(file);

	//build the URI
	var uri;
	if (file[0] == '/')
		uri = "/api/"+select+"/file"+file+"?func="+func;
	else
		uri = "/api/"+select+"/no-path-file/"+file+"?func="+func;
	
	$.get( uri, function(data) {
		success(data);
	})
	.fail(function(data) {
		fail();
	});
}

/********************************************************************/
NumaprofSourceEditor.prototype.moveToFile = function(file,func)
{
	//nothing to do
	if (this.file == file && !gblIsAsm)
	{
		this.doPostMove();
		return;
	}

	var tagsToReplace = {
	    '&': '&amp;',
	    '<': '&lt;',
	    '>': '&gt;'
	};

	function replaceTag(tag) {
	    return tagsToReplace[tag] || tag;
	}

	function safe_tags_replace(str) {
	    return str.replace(/[&<>]/g, replaceTag);
	}
	
	var margin = 150;
	
	//load the new file in editor
	console.log(file);
	if(file == '??' || file == '' || file == undefined) {
		// XHR fails to load file, show error message
		this.container.innerHTML = 
			'<pre class="line-numbers"><code>No source file to load</code></pre>';
		this.syntaxHighlighterEle = this.container.getElementsByTagName("code")[0];
		Prism.highlightElement(this.syntaxHighlighterEle);
		Prism.plugins.codeAnnotator.add(this.syntaxHighlighterEle, {
			line: 1, 
			text: "Error", 
			class: "line-annotate-large"
		});
		this.file = "";
		$("#"+this.containerId+" pre").height($( window ).height() - margin);
	} else {
		var cur = this;
		this.loadSourceFile(file,func,function(data){
			// File loaded, now highlight it
			cur.container.innerHTML = 
				'<pre class="line-numbers"><code class="' + 
				cur.getLanguageClassForHighlighter(cur.getLanguage(file)) + '">' +
				safe_tags_replace(data) + '</code></pre>';
			cur.syntaxHighlighterEle = cur.container.getElementsByTagName("code")[0];
			Prism.highlightElement(cur.syntaxHighlighterEle);
			cur.file = file;
			cur.updateAnotations(true,func);
			$("#"+cur.containerId+" pre").height($( window ).height() - margin);
		}, function() {
			// XHR fails to load file, show error message
			cur.container.innerHTML = 
				'<pre class="line-numbers"><code>Source for the file, ' 
				+ file + ', could not be loaded.</code></pre>';
			cur.syntaxHighlighterEle = cur.container.getElementsByTagName("code")[0];
			Prism.highlightElement(cur.syntaxHighlighterEle);
			Prism.plugins.codeAnnotator.add(cur.syntaxHighlighterEle, {
				line: 1, 
				text: "Error", 
				class: "line-annotate-large"
			});
			cur.file = "--";
			$("#"+cur.containerId+" pre").height($( window ).height() - margin);
		});
	}
	
	var cur = this;
	$( window ).resize(function() {
		$("#"+cur.containerId+" pre").height($( window ).height() - margin);
	});
}

/********************************************************************/
NumaprofSourceEditor.prototype.findLargestAnnot = function(file,func)
{
	var line = -1;
	var max = 0;

	for (var i in this.data)
	{
		//var value = this.data[i][mode];
		var value = this.selector.getValue(this.data[i]);

		if (value != undefined && this.data[i].file == file && this.data[i].func == func)
		{
			if (value > max)
			{
				max = value;
				line = i;
			}
		}
	}

	return line;
}

/********************************************************************/
NumaprofSourceEditor.prototype.moveToFileFunction = function(file,func)
{
	if (func != -1 && func != '' && func != '??')
	{
		this.postMove = {type:'func',func:func};
		$("#numaprof-source-filename").text(file+" | "+func);
	} else {
		$("#numaprof-source-filename").text(file);
		this.postMove = {};
	}
	this.moveToFile(file,func);
}

/****************************************************/
NumaprofSourceEditor.prototype.internalMergeStackMinMaxInfo = function(onto,value)
{
	onto.count += value.count;
	onto.sum += value.sum;
	if (onto.min == 0 || (value.min < onto.min && value.min != 0))
		onto.min = value.min;
	if (onto.max == 0 || (value.max > onto.max && value.max != 0))
		onto.max = value.max;
}

/****************************************************/
NumaprofSourceEditor.prototype.internalMergeStackInfoDatas = function(onto,value)
{
	onto.countZeros += value.countZeros;
	onto.maxAliveReq += value.maxAliveReq;
	onto.aliveReq += value.aliveReq;
	this.internalMergeStackMinMaxInfo(onto.alloc,value.alloc);
	this.internalMergeStackMinMaxInfo(onto.free,value.free);
	this.internalMergeStackMinMaxInfo(onto.lifetime,value.lifetime);
}

/********************************************************************/
NumaprofSourceEditor.prototype.internalComputeTotal = function(value)
{
	//already done
	if (value.total != undefined)
	{
		return;
	} else if (value.own == undefined) {
		value.total = jQuery.extend(true, {}, value.childs);
	} else {
		//copy
		value.total = jQuery.extend(true, {}, value.own);
		//merge
		if (value.childs != undefined)
			this.internalMergeStackInfoDatas(value.total,value.childs);
	}
}

/********************************************************************/
/**
 * Fix the relative paths by adding a string which is not removed by the browser,
 * opposite to './' which is removed on the http request.
 * @param {*} path 
 */
NumaprofSourceEditor.prototype.fixRelativePath = function(file)
{
	if (file[0] != '/')
		file = "./" + file;
	file = file.replace('./','/{.}/');
	return file;
}

/********************************************************************/
//extract max
NumaprofSourceEditor.prototype.extractMax = function(data)
{
	//setup some vars
	var max = 0;

	//loop on all datas
	for (var i in data)
	{
		var value = this.selector.getValue(data[i]);
		if (value != undefined)
		{
			if (value != undefined && value > max)
				max = value;
		}
	}
	
	//return max
	return max;
}

/********************************************************************/
//update anotations
NumaprofSourceEditor.prototype.updateAnotations = function(move,func)
{
	//keep track of current this
	var cur = this;
	var file = this.file;
	
	if (file == "??" || file == null || file == "")
		return;
	
	var select = "sources";
	if (gblIsAsm)
		select = "asm";

	//handle relative paths
	file = this.fixRelativePath(file);
	
	//build URI
	var uri;
	if (file[0] == '/')
		uri ="/api/"+select+"/file-stats"+file+"?func="+func;
	else
		uri ="/api/"+select+"/no-path-file-stats/"+file+"?func="+func;
		
	
	//fetch flat profile of current file
	$.get( uri, function(data) {
		//update data with more info than provided by server
		cur.data = data;
		for (var i in data)
		{
			data[i].file = cur.file;
			cur.internalComputeTotal(data[i]);
		}
		
		//draw annotations
		cur.redrawAnnotations();
		
		//move
		if (move != false)
			cur.doPostMove();
	})
	.fail(function(data) {
		numaprofHelper.logErrorRepl("Failed to load annotations for "+file);
	});
// 	maltDataSource.loadSourceFileAnnotations(file,function(data) {
// 
// 		//update data with more info than provided by server
// 		cur.data = data;
// 		for (var i in data)
// 		{
// 			data[i].file = cur.file;
// 			cur.internalComputeTotal(data[i]);
// 		}
// 		
// 		//draw annotations
// 		cur.redrawAnnotations();
// 		
// 		//move
// 		cur.doPostMove();
// 	});
}

/********************************************************************/
NumaprofSourceEditor.prototype.genPieDataTouch = function(data)
{
	var mode = this.selector.metric.split('.')[0];
	var dataset = [
		{legend:"Pinned", value:data.data[mode]["firstTouch"], color:"rgb(44, 160, 44)"},
		{legend:"Unpinned", value:data.data[mode]["unpinnedFirstTouch"], color:"rgb(255, 127, 14)"},
		];
	return dataset;
}

/********************************************************************/
NumaprofSourceEditor.prototype.genPieDataAccess = function(data)
{
	var mode = this.selector.metric.split('.')[0];
	var dataset = [
		{legend:"Local MCDRAM", value:data.data[mode]["localMcdramAccess"], color:"#FF79DE"},
		{legend:"Remote MCDRAM", value:data.data[mode]["remoteMcdramAccess"], color:"#54017a"},
		{legend:"Remote", value:data.data[mode]["remoteAccess"], color:"red"},
		{legend:"Local", value:data.data[mode]["localAccess"], color:"rgb(44, 160, 44)"},
		{legend:"Un. Both", value:data.data[mode]["unpinnedBothAccess"], color:"rgb(31, 119, 180)"},
		{legend:"Un. Thread", value:data.data[mode]["unpinnedThreadAccess"], color:"rgb(255, 127, 14)"},
		{legend:"Un. Page", value:data.data[mode]["unpinnedPageAccess"], color: "rgb(255, 187, 120)"},
		];
	return dataset;
}

/********************************************************************/
NumaprofSourceEditor.prototype.redrawAnnotations = function()
{
	//search max to compute color gradiant
	var max = this.extractMax(this.data);
	
	//use D3JS for color gradiant from blue -> red
	var colorScale = d3.scale.linear()
		.range(["#397EBA","#FF9595"])
		.domain([0,max]);
	
	//clear
	Prism.plugins.codeAnnotator.removeAll(this.syntaxHighlighterEle);

	//insert all markers
	var cur = this;
	for (var i in this.data) {
		if (this.selector.getValue(this.data[i]) != 0)
		Prism.plugins.codeAnnotator.add(this.syntaxHighlighterEle, {
			line: i, 
			text: this.selector.getFormattedValue(this.data[i]), 
			data: this.data[i],
			onPopoverTitle: function(data) {
				console.log(data);
				return gblIsAsm?"Line "+data.line+ " Addr 0x" + data.data.addr:"Line "+data.line;
			},
			onPopover: function(data) {
				var mode = cur.selector.metric.split('.')[0];
				var d = data.data[mode];
				var out = {};
				for (var i in d)
					out[i] = numaprofHelper.formatLongNumber(d[i]);
				var table = Mustache.render(cur.popoverTemplate, out);
				var charts = "<table class='annotation-details'><tr><th>Touch</th></tr><tr><td>"+cur.genD3Pie(cur.genPieDataTouch(data))+"</td></tr><tr><th>Access</th></tr><tr><td>"+cur.genD3Pie(cur.genPieDataAccess(data))+"</td></tr></table>";
				var full =  "<table><tr><td>"+table+"</td><td>"+charts+"</td></tr></table>";
				console.log(full);

				
				return $(full).html();
			},
			// class: "line-annotate-small",
			color: colorScale(this.selector.getValue(this.data[i])),
			onClick: function(ele, data) {
				cur.onClick(data.data)
			},
			data: this.data[i],
		});
	}
}

/*******************  FUNCTION  *********************/
NumaprofSourceEditor.prototype.genD3Pie = function(dataset)
{
// 		var dataset = [
// 		{legend:"apple", value:10, color:"red"},
// 		{legend:"orange", value:45, color:"orangered"},
// 		{legend:"banana", value:25, color:"yellow"},
// 		{legend:"peach", value:70, color:"pink"},
// 		{legend:"grape", value:20, color:"purple"}
// 		];

	var sum = 0;
	for (var i in dataset)
		sum += dataset[i].value;
	
	var width = 100;
	var height = width;
	var radius = width/2;
	
	$("#svg-buffer").html("<svg/>");

	var svg = d3.select("#svg-buffer svg")
		.attr("width", width)
		.attr("height", height)
		.append("g")
		.attr("transform", "translate(" + width / 2 + "," + height / 2 + ")");

	if (sum > 0)
	{
		var arc = d3.svg.arc()
			.outerRadius(radius)
			.innerRadius(00);

		var pie = d3.layout.pie()
			.sort(null)
			.value(function(d){ return d.value; });

		var g = svg.selectAll(".fan")
			.data(pie(dataset))
			.enter()
			.append("g")
			.attr("class", "fan")

		g.append("path")
			.attr("d", arc)
			.attr("fill", function(d){ return d.data.color; })
		
		g.append("text")
			.attr("transform", function(d) { return "translate(" + arc.centroid(d) + ")"; })
			.style("text-anchor", "middle")
			.style("font-size","12")
			.text(function(d) { return (d.data.value / sum > 0.2)?d.data.legend:""; });
	} else {
		svg.append("text")
			.attr("transform", function(d) { return "translate(0,0)"; })
			.style("text-anchor", "middle")
			.text(function(d) { return "NONE"; });
	}
	
	return $("#svg-buffer").html();
}
