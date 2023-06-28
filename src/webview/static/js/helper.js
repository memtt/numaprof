/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

function NumaprofHelper()
{
	this.POWER_PS  = ['&nbsp;','K','M','G','T','P'];
	this.POWER_NPS = [' ','K','M','G','T','P'];
	this.SUBPOWER_PS  = ['&nbsp;','m','u','n'];
	this.SUBPOWER_NPS = [' ','m','u','n','p'];
}

/*******************  FUNCTION  *********************/
NumaprofHelper.prototype.logError = function(message)
{
	$( "<div>" ).text(message).appendTo( "#errors" );
}

/*******************  FUNCTION  *********************/
NumaprofHelper.prototype.logErrorRepl = function(message)
{
	$("#errors").html("");
	$( "<div>" ).text(message).appendTo( "#errors" );
}

/*******************  FUNCTION  *********************/
NumaprofHelper.prototype.valOrDefault = function(value,defaultValue)
{
	if (value == undefined)
		return defaultValue;
	else
		return value;
}

/*******************  FUNCTION  *********************/
NumaprofHelper.prototype.formatLongNumber = function(value)
{
	var out = "";
	var tmp = value.toString();
	var cnt = 0;
	for (var i =  tmp.length - 1 ; i >= 0 ; i--)
	{
		if (cnt % 3 == 0)
			out = " " + out;
		out = tmp[i] + out;
		cnt++;
	}
	return out;
}

/*******************  FUNCTION  *********************/
NumaprofHelper.prototype.listToRange = function(elts)
{
	var start = -1;
	var last = -1;
	var out = [];
	
	//loop on all
	for (var i in elts)
	{
		if (start == -1)
		{
			start = +elts[i]
		} else if (elts[i] != last+1) {
			if (start != last)
				out.push(start+"-"+last);
			else
				out.push(start);
			start = +elts[i]
		}
		last = +elts[i]
	}

	//flush
	if (start != last)
		out.push(start+"-"+last);
	else
		out.push(start);
	
	//join
	return out.join(", ")
}

/*******************  FUNCTION  *********************/
/** Short helper to convert values to human readable format **/	
NumaprofHelper.prototype.humanReadable = function(value,decimals,unit,protectedSpace)
{
	var mul = 1000;
	if (unit == 'B' || unit == 'B')
		mul = 1024;
	
	if (value >= 1 && value < mul)
		decimals = 0;
	
	if (value == 0)
		decimals = 0;
	
	if (value >= 0.1 || value == 0)
	{
		var power = 0;
		while (value >= mul)
		{
			power++;
			value /= mul;
		}

		var res;
		if (protectedSpace)
			res = value.toFixed(decimals) + " " + this.POWER_PS[power] + unit;
		else
			res = value.toFixed(decimals) + " " + this.POWER_NPS[power] + unit;
	} else {
		var power = 0;
		while (value < 0.1 && power < 4)
		{
			power++;
			value *= 1000.0;
		}

		var res;
		if (protectedSpace)
			res = value.toFixed(decimals) + " " + this.SUBPOWER_PS[power] + unit;
		else
			res = value.toFixed(decimals) + " " + this.SUBPOWER_NPS[power] + unit;
	}

	return res;
}

/********************  GLOBALS  *********************/
numaprofHelper = new NumaprofHelper();
