//declare metrics
function NumaprofFuncMetrics()
{
}

NumaprofFuncMetrics.prototype.metrics = {
	'access.allFirstTouch': {
		name: 'All first touch',
		extractor: function(x) {return x.access.firstTouch + x.access.unpinnedFirstTouch;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum',
		subMetrics: ['access.firstTouch','access.unpinnedFirstTouch']
	},
	'access.allAccess': {
		name: 'All access',
		extractor: function(x) {return x.access.unpinnedPageAccess + x.access.unpinnedThreadAccess + x.access.unpinnedBothAccess + x.access.localAccess+x.access.remoteAccess+x.access.localMcdramAccess+x.access.remoteMcdramAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum',
		subMetrics: ['access.unpinnedPageAccess','access.unpinnedThreadAccess','access.unpinnedBothAccess','access.localAccess','access.remoteAccess','access.localMcdramAccess','access.remoteMcdramAccess']
	},
	'access.allMCRAMAccess': {
		name: 'All MCDRAM access',
		extractor: function(x) {return x.access.unpinnedPageAccess + x.access.unpinnedThreadAccess + x.access.unpinnedBothAccess + x.access.localAccess+x.access.remoteAccess+x.access.localMcdramAccess+x.access.remoteMcdramAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum',
		subMetrics: ['access.unpinnedPageAccess','access.unpinnedThreadAccess','access.unpinnedBothAccess','access.localAccess','access.remoteAccess','access.localMcdramAccess','access.remoteMcdramAccess']
	},
	'separator1': {
		name: '----------------------------------',
	},
	'alloc.allFirstTouch': {
		name: 'All alloc first touch',
		extractor: function(x) {return x.alloc.firstTouch + x.alloc.unpinnedFirstTouch;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum',
		subMetrics: ['alloc.firstTouch','alloc.unpinnedFirstTouch']
	},
	'alloc.allAccess': {
		name: 'All alloc access',
		extractor: function(x) {return x.alloc.unpinnedPageAccess + x.alloc.unpinnedThreadAccess + x.alloc.unpinnedBothAccess + x.alloc.localAccess+x.alloc.remoteAccess+x.alloc.localMcdramAccess+x.alloc.remoteMcdramAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum',
		subMetrics: ['alloc.unpinnedPageAccess','alloc.unpinnedThreadAccess','alloc.unpinnedBothAccess','alloc.localAccess','alloc.remoteAccess','alloc.localMcdramAccess','alloc.remoteMcdramAccess']
	},
	'separator2': {
		name: '----------------------------------',
	},
	'access.firstTouch': {
		name: 'First touch',
		extractor: function(x) {return x.access.firstTouch;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'access.unpinnedFirstTouch': {
		name: 'Unpinned first touch',
		extractor: function(x) {return x.access.unpinnedFirstTouch;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'access.unpinnedPageAccess': {
		name: 'Unpinned page access',
		extractor: function(x) {return x.access.unpinnedPageAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'access.unpinnedThreadAccess': {
		name: 'Unpinned thread access',
		extractor: function(x) {return x.access.unpinnedThreadAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'access.unpinnedBothAccess': {
		name: 'Unpinned both access',
		extractor: function(x) {return x.access.unpinnedBothAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'access.localAccess': {
		name: 'Local accecss',
		extractor: function(x) {return x.access.localAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'access.remoteAccess': {
		name: 'Remote access',
		extractor: function(x) {return x.access.remoteAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'access.localMcdramAccess': {
		name: 'Local MCDRAM access',
		extractor: function(x) {return x.access.localMcdramAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'access.remoteMcdramAccess': {
		name: 'Remote MCDRAM access',
		extractor: function(x) {return x.access.remoteMcdramAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'separator3': {
		name: '----------------------------------',
	},
	'alloc.firstTouch': {
		name: 'Alloc first touch',
		extractor: function(x) {return x.alloc.firstTouch;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'alloc.unpinnedFirstTouch': {
		name: 'Alloc unpinned first touch',
		extractor: function(x) {return x.alloc.unpinnedFirstTouch;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'alloc.unpinnedPageAccess': {
		name: 'Alloc unpinned page access',
		extractor: function(x) {return x.alloc.unpinnedPageAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'alloc.unpinnedThreadAccess': {
		name: 'Alloc unpinned thread access',
		extractor: function(x) {return x.alloc.unpinnedThreadAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'alloc.unpinnedBothAccess': {
		name: 'Alloc unpinned both access',
		extractor: function(x) {return x.alloc.unpinnedBothAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'alloc.localAccess': {
		name: 'Alloc local accecss',
		extractor: function(x) {return x.alloc.localAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'alloc.remoteAccess': {
		name: 'Alloc remote access',
		extractor: function(x) {return x.alloc.remoteAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'alloc.localMcdramAccess': {
		name: 'Local alloc MCDRAM access',
		extractor: function(x) {return x.alloc.localMcdramAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
	'alloc.remoteMcdramAccess': {
		name: 'Remote alloc MCDRAM access',
		extractor: function(x) {return x.alloc.remoteMcdramAccess;},
		formalter: function(x) {return numaprofHelper.humanReadable(x,1,'',false);},
		defaultOrder: 'desc',
		ref: 'sum'
	},
};

NumaprofFuncMetrics.prototype.getMetricList = function()
{
	var ret = [];
	for (var i in this.metrics)
	{
		ret.push({name:this.metrics[i].name,key:i});
	}
	return ret;
}

NumaprofFuncMetrics.prototype.getMetricNames = function()
{
	var res = [];
	for (var i in this.metrics)
		res.push(this.metrics[i].name);
	return res;
}

NumaprofFuncMetrics.prototype.getValue = function(dataElement,metricName,inclusive)
{
	var metric = this.metrics[metricName];
	
	if (dataElement == undefined)
	{
		return 0;
	} else {
		return metric.extractor(dataElement);
	}

	/*if (dataElement == undefined)
	{
		return 0;
	} else if (inclusive) {
		return metric.extractor(dataElement.total);
	} else if (dataElement.own == undefined) {
		return 0;
	} else {
		return metric.extractor(dataElement.own);
	}*/
}

NumaprofFuncMetrics.prototype.getFormattedValue = function(dataElement,metricName,inclusive)
{
	return this.metrics[metricName].formalter(this.getValue(dataElement,metricName,inclusive));
}

NumaprofFuncMetrics.prototype.getValueRatio = function(dataElement,metricName,inclusive)
{
	return (100 *this.getValue(dataElement,metricName,inclusive)) / this.ref[metricName];
}

NumaprofFuncMetrics.prototype.getRef = function(data,metricName)
{
// 	if (this.refs == undefined)
// 		this.buildRefs(data);
// 	console.log(metricName + " " + this.refs[metricName]);
// 	return this.refs[metricName];
	return this.computeRef(data,metricName);
}

NumaprofFuncMetrics.prototype.buildRefs = function(data)
{
	this.refs = {};
	for (var i in this.metrics)
		this.refs[i] = this.computeRef(data,this.metrics[i]);
}

NumaprofFuncMetrics.prototype.computeRef = function(data,metricName)
{
	var metric = this.metrics[metricName];
	var res = 0;
	switch(metric.ref)
	{
		case 'sum':
			for (var i in data)
				res += this.getValue(data[i],metricName);
			break;
		case 'max':
			for (var i in data)
			{
				var tmp = this.getValue(data[i],metricName);
				if (tmp > res)
					res = tmp;
			}
			break;
		default:
			console.log("Invalid value for ref mode for metric "+metricName+" : "+metric.ref);
			res = 1;
	}
	return res;
}

// If this is ran on server-side, we export the class as a module
// Otherwise, we create a globally shared object for this class
if(typeof exports !== 'undefined') {
	module.exports = NumaprofFuncMetrics;
}
