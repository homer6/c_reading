-- converts animation frames to seconds
function this.frames( this, count )
	return count / 30
end

function this.testF( this )
	trace( "testF" )
end

function this.jumpStart( this )
	this:jump( this:frames(20), 10 )
	this:blendTo( "jump", this:frames(10) )
end

function this.accelerateStart( this )
end

function this.f( this, i )
	this:testF()

	while ( i <= 4 ) do
		this:hello1()
		this:hello2( i )
		i = i+1
	end
	
	trace( "Hello, debug world!" );
end
