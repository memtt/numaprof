/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

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
						<div class='progress'>\
							<div class='progress-bar' role='progressbar' aria-valuenow='60' aria-valuemin='0' aria-valuemax='100' style='width: {{ ratio }}%;'>\
								<span class='sr-only'>{{ ratio }}% Complete</span>\
							</div>\
						</div>\
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
	var sourceEditor = new MaltSourceEditor('malt-source-editor',undefined);
	sourceEditor.moveToFile("/home/sebv/Projects/numaprof/src/core/ProcessTracker.cpp");
});
