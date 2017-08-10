function NumaprofSelector()
{
	this.metrics = new NumaprofFuncMetrics();
	this.metric = 'access.allAccess';
	this.inclusive = true;
	this.limit = 10;
	this.ratio = false;
	this.query='';
	this.order = this.metrics.metrics[this.metric].defaultOrder;
	this.functions = [];
	this.currentPage = 1;
	this.perPage = 10;
	this.totalElements = 0;
}

NumaprofSelector.prototype.getMetrics = function()
{
	return this.metrics.metrics;
}

NumaprofSelector.prototype.getMetricName = function()
{
	return this.metrics.metrics[this.metric].name;
}

NumaprofSelector.prototype.setData = function(data)
{
	this.functions = data;
	this.onInternalChange();
}

NumaprofSelector.prototype.onInternalChange = function()
{
	var cnt = 0;
	for(var i in this.functions)
	{
		if (this.filter(this.functions[i]))
			cnt++;
	}
	this.totalElements = cnt;
	console.log(cnt);
	this.onChange();
}

NumaprofSelector.prototype.getValue = function(x)
{
	return this.metrics.getValue(x,this.metric,this.inclusive);
}

NumaprofSelector.prototype.computeRef = function() 
{
	return this.metrics.getRef(this.functions,this.metric);
}

NumaprofSelector.prototype.getValueRatio = function(x) 
{
	return (100 *this.getValue(x)) / this.computeRef();
}

NumaprofSelector.prototype.getFormattedValue = function(x) 
{
	if (this.ratio)
	{
		return this.getValueRatio(x).toFixed(1)+"%";
	} else {
		return this.metrics.getFormattedValue(x,this.metric,this.inclusive);
	}
}

NumaprofSelector.prototype.filter = function(x) 
{
	return (this.getValue(x) > 0 && (this.query == '' || x.function.indexOf(this.query) > -1));
}

NumaprofSelector.prototype.isReversedOrder = function () 
{
	return (this.order == 'desc');
}

NumaprofSelector.prototype.accepted = function(x)
{
	return (this.getValue(x) > 0 && (this.query == '' || x.function.indexOf(this.query) > -1));
}

NumaprofSelector.prototype.toogleOrder = function()
{
	this.order = (this.order == 'asc')?'desc':'asc';
	this.onInternalChange();
}

NumaprofSelector.prototype.toogleRatio = function()
{
	this.ratio =  !this.ratio;
	this.onInternalChange();
}

NumaprofSelector.prototype.toogleInclusive = function()
{
	this.inclusive = !this.inclusive;
	this.onInternalChange();
}

NumaprofSelector.prototype.getCurMetricName = function()
{
	return this.metrics.metrics[this.metric].name;
}

NumaprofSelector.prototype.selectMetric = function(metric)
{
	this.metric = metric;
	this.order = this.metrics.metrics[this.metric].defaultOrder;
	this.onInternalChange();
}

NumaprofSelector.prototype.onChange = function()
{
	
}
