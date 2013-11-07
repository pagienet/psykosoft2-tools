package 
{
	import flash.events.Event;
	import flash.net.LocalConnection;
	import flash.utils.ByteArray;
	
	import be.aboutme.airserver.AIRServer;
	import be.aboutme.airserver.endpoints.udp.UDPEndPoint;
	import be.aboutme.airserver.events.AIRServerEvent;
	import be.aboutme.airserver.events.MessageReceivedEvent;
	import be.aboutme.airserver.messages.Message;
	import be.aboutme.airserver.messages.serialization.NativeObjectSerializer;
	

	public class PsykoServer
	{
		private var server:AIRServer;
		private var errorCallback:Function;
		private var callbackObject:Object;
		private var logCallback:Function;
		private var messageCallback:Function;
		private var sendCallback:Function;
		private var partialBlocks:Vector.<DataBlock>;
		
		public function PsykoServer( udpPort:uint = 1236, callbackObject:Object = null, messageCallback:Function  = null,logCallback:Function  = null, errorCallback:Function  = null, sendCallback:Function  = null)
		{
			this.callbackObject = callbackObject;
			this.messageCallback = messageCallback;
			this.logCallback = logCallback;
			this.errorCallback = errorCallback;
			this.sendCallback = sendCallback;
			init( udpPort );
		}
		
		private function init(udpPort:uint):void
		{
			partialBlocks = new Vector.<DataBlock>();
			
			server = new AIRServer();
			//add the listening endpoints
			/*
			server.addEndPoint(new SocketEndPoint(1233, new PlainTextSocketClientHandlerFactory()));
			server.addEndPoint(new SocketEndPoint(1234, new AMFSocketClientHandlerFactory()));
			server.addEndPoint(new SocketEndPoint(1235, new WebSocketClientHandlerFactory()));
			*/
			server.addEndPoint(new UDPEndPoint(udpPort, new NativeObjectSerializer(), 60000));
			
			//add event listeners
			server.addEventListener(AIRServerEvent.CLIENT_ADDED, clientAddedHandler, false, 0, true);
			server.addEventListener(AIRServerEvent.CLIENT_REMOVED, clientRemovedHandler, false, 0, true);
			server.addEventListener(MessageReceivedEvent.MESSAGE_RECEIVED, messageReceivedHandler, false, 0, true);
			//start the server
			try
			{
				server.start();
			} catch ( error:Error )
			{
				sendError( error.message );
			}
			
		}
		
		private function exitingHandler(event:Event):void
		{
			server.stop();
		}
		
		private function clientAddedHandler(event:AIRServerEvent):void
		{
			sendLog("Client added: " + event.client.id + "\n");
			if ( callbackObject && messageCallback )
			{
				messageCallback.apply( callbackObject, ['<msg src="Client.added"/>'] );
			}
		}
		
		private function clientRemovedHandler(event:AIRServerEvent):void
		{
			sendLog("Client removed: " + event.client.id + "\n");
			if ( callbackObject && messageCallback )
			{
				messageCallback.apply( callbackObject, ['<msg src="Client.removed"/>'] );
			}
		}
		
		private function messageReceivedHandler(event:MessageReceivedEvent):void
		{
			
			sendLog("<client" + event.message.senderId + "> " + event.message.data + "\n");
			if ( !(event.message.command == "PORT" || event.message.command == "ADDR" ) && callbackObject && messageCallback )
			{
				if ( event.message.data is ByteArray )
				{
					var ba:ByteArray = (event.message.data as ByteArray);
					ba.position = 0;
					var id:uint = ba.readUnsignedInt();
					var blockID:uint = ba.readUnsignedInt();
					var totalSize:uint = ba.readUnsignedInt();
					var totalPackages:uint = ba.readUnsignedInt();
					var offset:uint = ba.readUnsignedInt();
					var partialBlock:DataBlock = getPartialBlock(blockID);
					if ( partialBlock == null ) {
						partialBlock = new DataBlock(blockID, totalSize, totalPackages);
						partialBlocks.push(partialBlock);
					}
					var isComplete:Boolean = partialBlock.addPacket(offset, ba );
					sendString( <msg ack={id} />.toXMLString());
					if ( isComplete )
					{
						partialBlock.data.position = 0;
						var result:String = "";
						while ( partialBlock.data.bytesAvailable > 0 )
						{
							result += String.fromCharCode( partialBlock.data.readByte() );
						}
						messageCallback.apply( callbackObject, [result] );
						removePartialBlock(blockID);
					}
					
				} else {
					messageCallback.apply( callbackObject, [event.message.data] );
				}
			}
		}
		
		private function getPartialBlock(blockID:uint):DataBlock
		{
			for ( var i:int = 0; i < partialBlocks.length; i++ )
			{
				if ( partialBlocks[i].blockID == blockID ) return  partialBlocks[i];
			}
			return null;
		}
		
		private function removePartialBlock(blockID:uint):void
		{
			for ( var i:int = 0; i < partialBlocks.length; i++ )
			{
				if ( partialBlocks[i].blockID == blockID ) 
				{
					partialBlocks.splice(i,1);
					break;
				}
			}
		}
		
		private function sendLog(message:String):void
		{
			if ( callbackObject && logCallback )
			{
				logCallback.apply( callbackObject, [message] );
			}
			
		}
		
		private function sendError(message:String):void
		{
			if ( callbackObject && errorCallback )
			{
				errorCallback.apply( callbackObject, [message] );
			}
			
		}
		
		public function sendString( data:String):void
		{
			var message:Message = new Message();
			message.data = data;
			server.sendMessageToAllClients(message);
			if ( callbackObject && logCallback )
			{
				sendCallback.apply( callbackObject, [data] );
			}
		}
	}
}