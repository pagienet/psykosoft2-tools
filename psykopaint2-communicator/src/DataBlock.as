package
{
	import flash.utils.ByteArray;
	import flash.utils.Dictionary;

	public class DataBlock
	{
		public var blockID:uint;
		public var data:ByteArray;
		private var blocksMissing:int;
		private var blocksReceived:Dictionary;
		
		public function DataBlock( id:uint, totalSize:uint, totalBlocks:uint )
		{
			blockID = id;
			data = new ByteArray();
			data.length = totalSize;
			blocksMissing = totalBlocks;
			blocksReceived = new Dictionary();
		}
		
		public function addPacket( offset:uint, data:ByteArray ):Boolean
		{
			if ( blocksReceived[offset] == null )
			{
				blocksReceived[offset] = true;
				this.data.position = offset;
				this.data.writeBytes(data,data.position);
				blocksMissing--;
			}
			return blocksMissing == 0;
		}
	}
}