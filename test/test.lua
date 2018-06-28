
require("luapb");
print('------------start------------------- ' )

pb.set_log( 3 )        	--LOG_ERROR
pb.register_path( "./" )
pb.register_file( "persion.proto" )

pb_buf = pb.encode( "Person", { name="test1", id=104, email="163.com"，phone={ {number=444,type=1},{ number=5555,type="WORK" } } }   )
pb_msg = pb.decode( "Person", pb_buf    )

local function dump( tb )
	for k, v in pairs( tb ) do
		if type(v) == 'table' then
			dump(v )
		else 
		   print( k, v )
		end
	end 
end 
dump（ pb_msg ）

print('-----------------end------------------- ' )
