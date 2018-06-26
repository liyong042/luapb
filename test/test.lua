
require("luapb");

print('------------start------------------- ' )

pb.set_log( 3 )        	--LOG_ERROR
pb.register_path( "./" )
pb.register_file( "persion.proto" )

pb_buf = pb.encode( "Person", { name="test1", id=104, email="163.com" }   )
pb_msg = pb.decode( "Person", pb_buf    )

for k, v in pairs( pb_msg ) do
	print( k, v )
end 

print('-----------------end------------------- ' )
