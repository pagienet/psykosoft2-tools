package
{
	import com.bit101.components.CheckBox;
	import com.bit101.components.ComboBox;
	import com.bit101.components.HBox;
	import com.bit101.components.HRangeSlider;
	import com.bit101.components.HSlider;
	import com.bit101.components.HUISlider;
	import com.bit101.components.InputText;
	import com.bit101.components.Knob;
	import com.bit101.components.Label;
	import com.bit101.components.Panel;
	import com.bit101.components.PushButton;
	import com.bit101.components.RangeSlider;
	import com.bit101.components.RotarySelector;
	import com.bit101.components.ScrollPane;
	import com.bit101.components.TextArea;
	import com.bit101.components.VBox;
	import com.bit101.components.Window;
	
	import flash.display.DisplayObjectContainer;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.net.InterfaceAddress;
	import flash.net.NetworkInfo;
	import flash.net.NetworkInterface;
	import flash.utils.Dictionary;
	import flash.utils.getTimer;
	
	import net.psykosoft.psykopaint2.core.drawing.data.PsykoParameter;
	
	public class Psykommunicator extends Sprite
	{
		private var psykoSocket:PsykoServer;
		private var uiHolder:Sprite;
		private var errorMessages:TextArea;
		private var logMessages:TextArea;
		private var sendMessages:TextArea;
		
		private var brushUI:Sprite;
		private var uiElementToParameter:Dictionary;
		private var inputToSlider:Dictionary;
		private var sliderToInput:Dictionary;
		private var decoratorList:ComboBox;
		private var shapeList:ComboBox;
		
		private var addDecoratorBtn:PushButton;
		private var decoratorPanels:Vector.<Window>;
		private var decoratorPanel:Panel;
		private var lastPing:int;
		private var brushShapes:ComboBox;
		private var shapesAvailable:Boolean;
		private var otherMessages:TextArea;

		private var currentBrushPane:ScrollPane;
		private var penWindow:Window;
		private var penInfo:TextArea;
		private var penPressure:int = -1;
		private var penButton1Down:Boolean = false;
		private var penButton2Down:Boolean = false;
		private var availableShapesRequestedTime:int = -1;
		private var availableDecoratorsRequestedTime:int = -1;
		private var decoratorsAvailable:Boolean;
		private var messageSelector:ComboBox;
		private var messageSender:PushButton;
		
		
		
		public function Psykommunicator()
		{
			addEventListener(Event.ADDED_TO_STAGE, onAddedToStage );
		}
		
		private function onAddedToStage( event:Event ):void
		{
			uiElementToParameter = new Dictionary(true);
			inputToSlider = new Dictionary(true);
			sliderToInput = new Dictionary(true);
			initUI();
			initSocket();
		}
		
		private function initSocket():void
		{
			// TODO Auto Generated method stub
			lastPing = -1;
			psykoSocket = new PsykoServer(1236,this,onMessage, onLog,onError,onSend);
			getIps();
		}
		
		private function getIps():void
		{
			var netInterfaces:Vector.<NetworkInterface> = NetworkInfo.networkInfo.findInterfaces();
			if (netInterfaces && netInterfaces.length > 0) {    
				for each (var i:NetworkInterface in netInterfaces) {
					if (i.active) {
						var addresses:Vector.<InterfaceAddress> =i.addresses;
						for each (var j:InterfaceAddress in addresses) {
							logMessages.text += "Server IP: "+ j.address+"\n";
							return;
						}
					}
				}
			}
		}  
		
		private function onError( message:String ):void
		{
			//trace( message );
			logMessages.text += message;
		}
		
		private function onLog( message:String ):void
		{
			logMessages.text += message;
			
		}
		
		private function onSend( message:String ):void
		{
			sendMessages.text += "\n"+message;
			sendMessages.textField.scrollV = sendMessages.textField.maxScrollV;
		}
		
		private function onMessage( message:*):void
		{
			if ( message is String )
			{
				try
				{
					var xml:XML = new XML( String(message) );
					if ( xml.hasOwnProperty("@src") )
					{
						processMessage( xml );
					}
				} catch (e:Error )
				{
					trace(e.message);
				}
			} else if ( message is int )
			{
				if ( message < lastPing || lastPing == -1 )
				{
					resync();
					
				} 
				lastPing = message;
			}
			
		}
		
		private function processMessage(xml:XML):void
		{
			var src:String = String(xml.@src);
			if ( src== "ActiveBrushKit.parameterSet" )
			{
				showActiveBrushPanel( xml );
			}
			else if ( src == "PointDecoratorFactory.sendAvailableDecorators" )
			{
				showAvailablePointDecorators( xml )	
			}
			else if ( src == "BrushShapeLibrary.sendAvailableShapes" )
			{
				showAvailableShapes( xml )	
			}
			else if ( src == "Client.added" )
			{
				//clearActiveBrushPanel();
				//resync();
			}
			else if ( src ==  "Client.removed" )
			{
				clearActiveBrushPanel();
			}
			else if ( src ==  "PaintModule.onMemoryWarning" )
			{
				showMemoryWarning();
			}else if ( src ==  "PaintModule.activate" )
			{
				clearActiveBrushPanel();
				resync();
			}  
			else if ( src.indexOf("Pen") == 0 )
			{
				handlePenMessage(xml)
			}else {
				showUnhandledMessage(xml);
				
			}
		}
		
		private function handlePenMessage(xml:XML):void
		{
			if ( xml.@src == "Pen.onPressureChanged" )
			{
				penPressure = int( xml.@pressure );
			} else if ( xml.@src == "Pen.onButton1Pressed" )
			{
				penButton1Down = true;
			}else if ( xml.@src == "Pen.onButton2Pressed" )
			{
				penButton2Down = true;
			}else if ( xml.@src == "Pen.onButton1Released" )
			{
				penButton1Down = false;
			}else if ( xml.@src == "Pen.onButton2Released" )
			{
				penButton2Down = false;
			}
			
			penInfo.text = "Pressure: "+penPressure+"\n"+
				           (penButton1Down ? "Button 1 pressed" : "")+"\n"+
						   (penButton2Down ? "Button 2 pressed" : "")+"\n";
				
				
			
			
		}
		
		private function showUnhandledMessage(xml:XML):void
		{
			otherMessages.text += xml.toXMLString()+"\n";
			otherMessages.textField.scrollV = otherMessages.textField.maxScrollV;
			
		}
		
		private function showMemoryWarning():void
		{
			otherMessages.text += "!!!!!!!!!!! MEMORY WARNING !!!!!!!!!!!!!";
			
		}
		
		private function resync():void
		{
			var request:String = "";
			if ( !shapesAvailable && getTimer() - availableShapesRequestedTime > 1000 )
			{
				request += "BrushShapeLibrary.getAvailableShapes;";
				availableShapesRequestedTime = getTimer();
			}
			
			if ( !decoratorsAvailable && getTimer() - availableDecoratorsRequestedTime > 1000 )
			{
				request += "PointDecoratorFactory.getAvailableDecorators;";
				availableShapesRequestedTime = getTimer();
			}
			request += "ActiveBrushKit.getParameterSet";
			var message:XML = <msg target={request} />;
			psykoSocket.sendString( message.toXMLString() );
		}
		
		private function showAvailablePointDecorators(xml:XML):void
		{
			decoratorsAvailable = true;
			var items:Array = [];
			for ( var i:int = 0; i < xml.decorator.length(); i++ )
			{
				items.push( String( xml.decorator[i].@id));
			}
			decoratorList.items = items;
		}
		
		private function showAvailableShapes(xml:XML):void
		{
			shapesAvailable = true;
			var items:Array = [];
			for ( var i:int = 0; i < xml.shape.length(); i++ )
			{
				items.push( String( xml.shape[i].@id));
			}
			shapeList.items = items;
		}
		
		private function clearActiveBrushPanel():void
		{
			while ( brushUI.numChildren > 0 ) 
			{
				uiElementToParameter[brushUI.removeChildAt(0)] = null;
			}
		}
		
		private function showActiveBrushPanel(xml:XML):void
		{
			/*
			if ( !shapesAvailable )
			{
				resync();
			}
			*/
			xml = xml.brush[0];
			clearActiveBrushPanel();	
			currentBrushPane = new ScrollPane( brushUI,0,0 );
			currentBrushPane.width = 750;
			currentBrushPane.height = stage.stageHeight - currentBrushPane.y;
			currentBrushPane.autoHideScrollBar = true;
			var vb:VBox = new VBox( currentBrushPane.content,0,0 );
			vb.width = 750;
			
			decoratorPanels = new Vector.<Window>();
			
			if ( xml.parameter.length() > 0 )
			{
				createParameterBlock( vb, xml.parameter, String(xml.@id), false);
			} 
			
			/*
			if ( xml.shapes.length() > 0 )
			{
				createShapeBlock( vb, xml.shapes[0] );
			}
			*/
				
			if (  xml.pathengine.length() > 0 )
			{
				if ( xml.pathengine[0].parameter.length() > 0 )
				{
					createParameterBlock( vb, xml.pathengine[0].parameter,"Path Engine", false);
				} 
				if ( !vb.contains(decoratorPanel) ) vb.addChild( decoratorPanel );
				
				for ( var i:int = 0; i < xml.pathengine[0].children().length(); i++ )
				{
					var subBlock:XML =  xml.pathengine[0].children()[i];
					if (  subBlock.name().toString() != "parameter" )
					{
						createParameterBlock( vb, subBlock.parameter, subBlock.name().toString(), true);
					}
				}
				
			}
			
			updateUpDownButtons();
			addDecoratorBtn.enabled = decoratorList.items != null
			
		}
		
		private function createShapeBlock( parent:DisplayObjectContainer, shapeData:XML):void
		{
			if ( shapeData.shape.length() == 0 ) return;
			
			var window:Window = new Window(parent, 0, 0, "Brush Shapes");
			window.width = 700;
			window.height = 40;
			window.hasMinimizeButton = true;
			window.draggable = false;
			window.shadow = false;
			
			parent.setChildIndex(window,0);
			
			
			var shapeItems:Array = [];
			for ( var i:int = 0; i < shapeData.shape.length(); i++ )
			{
				shapeItems.push( shapeData.shape[i].@type );
			}
			var hb:HBox = new HBox(window.content,0,0)
			brushShapes = new ComboBox(hb,0,0,"Brush Shapes",shapeItems );
			var pb:PushButton = new PushButton(hb,0,0,"Remove Selected Shape", onRemoveSelectedShape );
			pb.width = 120;
			hb.addChild(shapeList);
			new PushButton(hb,0,0,"Add Shape", onAddSelectedShape );
			
		}
		
		private function onRemoveSelectedShape( event:Event ):void
		{
			var message:XML = <msg target="ActiveBrushKit.removeShapeAtIndex" index={brushShapes.selectedIndex}/>;
			psykoSocket.sendString( message.toXMLString() );
			
		}
		
		private function onAddSelectedShape(event:Event ):void
		{
			var message:XML = <msg target="ActiveBrushKit.addShape" type={shapeList.selectedItem}/>;
			psykoSocket.sendString( message.toXMLString() );
			
		}
		
		
		private function createParameterBlock( parent:DisplayObjectContainer, parameterData:XMLList, title:String, closeable:Boolean):void
		{
			var window:Window = new Window(parent, 0, 0, title);
			window.width = 700;
			window.hasMinimizeButton = true;
			window.draggable = false;
			window.shadow = false;
			if ( closeable ) {
				
				var hb:HBox = new HBox(window.titleBar,535,4);
				hb.name = "controlHolder";
				var b:PushButton = new PushButton(hb,0,0,"UP",onPanelUp);
				b.width = 50;
				b.height = 12;
				b.name = "up";
				
				b = new PushButton(hb,0,0,"DOWN",onPanelDown);
				b.width = 50;
				b.height = 12;
				b.name = "down";
				
				b = new PushButton(hb,0,0,"DELETE",onPanelClose);
				b.width = 50;
				b.height = 12;
				
				decoratorPanels.push(window);
			}
			
			var holder:HBox = new HBox(window.content,4,4);
			
			var labelBox:VBox = new VBox(holder,0,0);
			labelBox.spacing = 3;
			var sliderBox:VBox = new VBox(holder,0,0);
			sliderBox.spacing = 5;
			sliderBox.width = 400;
			var inputBox:VBox = new VBox(holder,0,0);
			inputBox.spacing = 5;
			inputBox.width = 50;
			var uiBox:VBox = new VBox(holder,0,3);
			uiBox.spacing = 11;
			
			for ( var i:int = 0; i < parameterData.length(); i++ )
			{
				var parameterXML:XML = parameterData[i];
				if ( String( parameterXML.name()) == "parameter" )
				{
					var parameterType:int = int( parameterXML.@type );
					var lbl:Label = new Label(labelBox,0,0,parameterXML.@id);
					lbl.height = 16;
					
					if ( parameterType==PsykoParameter.NumberParameter || parameterType==PsykoParameter.IntParameter || parameterType==PsykoParameter.AngleParameter)
					{
						var slider:HSlider = new HSlider( sliderBox,0,0,onSliderChanged );
						slider.width = 400;
						slider.height = 16;
						slider.minimum = Number( parameterXML.@minValue );
						slider.maximum = Number( parameterXML.@maxValue );
						slider.value = Number( parameterXML.@value );
						slider.tick = ( parameterType == PsykoParameter.IntParameter ? 1 : 0.001 );
						uiElementToParameter[slider] = parameterXML;
						var t:InputText = new InputText(inputBox,0,0,slider.value.toString(), onInputTextChanged );
						t.width = 50;
						inputToSlider[t] = slider;
						sliderToInput[slider] = t;
					} else if( parameterType==PsykoParameter.NumberRangeParameter || parameterType ==PsykoParameter.IntRangeParameter || parameterType==PsykoParameter.AngleRangeParameter )
					{
						var rangeSlider:RangeSlider = new HRangeSlider( sliderBox,0,3,onRangeSliderChanged );
						rangeSlider.width = 400;
						rangeSlider.height = 16;
						rangeSlider.minimum = Number( parameterXML.@minValue );
						rangeSlider.maximum = Number( parameterXML.@maxValue );
						rangeSlider.lowValue = Number( parameterXML.@value1 );
						rangeSlider.highValue = Number( parameterXML.@value2 );
						rangeSlider.tick = ( parameterType == PsykoParameter.IntRangeParameter ? 1 : 0.00001 );
						rangeSlider.labelPrecision = ( parameterType == PsykoParameter.IntRangeParameter ? 0 : 5 );
						rangeSlider.labelMode = RangeSlider.NEVER;
						uiElementToParameter[rangeSlider] = parameterXML;
						hb = new HBox(inputBox,0,0);
						var t1:InputText = new InputText(hb,0,0,rangeSlider.lowValue.toString(), onInputTextChanged );
						t1.name = "low";
						t1.width = 50;
						var t2:InputText = new InputText(hb,0,0,rangeSlider.highValue.toString(), onInputTextChanged );
						t2.width = 50;
						t2.name = "high";
						inputToSlider[t1] = rangeSlider;
						inputToSlider[t2] = rangeSlider;
						sliderToInput[rangeSlider] = [t1,t2];
					} else if (parameterType == PsykoParameter.BooleanParameter) 
					{	
						var bb:CheckBox = new CheckBox(sliderBox,0,6,"",onCheckBoxChanged );
						bb.height = 16;
						bb.selected = int( parameterXML.@value ) == 1;
						uiElementToParameter[bb] = parameterXML;
						var dummy:Label = new Label(inputBox,0,0," ");
						dummy.height = 16;
					} else if (parameterType == PsykoParameter.StringListParameter || parameterType == PsykoParameter.IconListParameter || parameterType == PsykoParameter.IntListParameter ) 
					{	
						var cbb:ComboBox = new ComboBox(sliderBox,0,0,"", (parameterXML.@list).split(","));
						cbb.addEventListener(Event.SELECT, onComboboxChanged );
						cbb.selectedIndex = int( parameterXML.@index );
						uiElementToParameter[cbb] = parameterXML;
						dummy = new Label(inputBox,0,0," ");
						dummy.height = 16;
					}  else if (parameterType == PsykoParameter.ColorParameter ) 
					{	
						var t1:InputText = new InputText(sliderBox,0,0,"color", onInputTextChanged );
						uiElementToParameter[t1] = parameterXML;
						dummy = new Label(inputBox,0,0," ");
						dummy.height = 16;
					} else {	
						throw( "implement parameter type "+PsykoParameter.getTypeName(parameterType));
							
					}
				}
				var cb:CheckBox = new CheckBox( uiBox,0,3,"User UI",onShowInUI );
				if ( parameterXML.hasOwnProperty("@showInUI") )
				{
					cb.selected = parameterXML.@showInUI == "1" ;
				} else {
					cb.selected = false;
				}
				uiElementToParameter[cb] = parameterXML;
			}
			if (  parameterData.length() == 1 )
			{
				new Label(labelBox,0,0);
				new Label(sliderBox,0,0);
				new Label(inputBox,0,0);
				new Label(uiBox,0,0);
			}
			inputBox.width = 105;
			holder.height =parameterData.length() * 20 ;
			 window.height = holder.height + 32;
		}
		
		
		private function onPanelDown(event:Event):void
		{
			var w:Window = event.target.parent.parent.parent.parent as Window;
			var p:DisplayObjectContainer = w.parent;
			var parentIndex:int = p.getChildIndex(w);
			var decoratorIndex:int = decoratorPanels.indexOf(w);
			p.removeChild(w);
			decoratorPanels.splice(decoratorIndex,1);
			decoratorPanels.splice(decoratorIndex+1,0,w);
			p.addChildAt(w,parentIndex+1);
			
			var message:XML = <msg target="ActiveBrushKit.movePointDecoratorIndex" oldIndex={decoratorIndex}  newIndex={decoratorIndex+1}/>;
			psykoSocket.sendString( message.toXMLString() );
			
			updateUpDownButtons();
		}
		
		
		
		private function onPanelUp(event:Event):void
		{
			var w:Window = event.target.parent.parent.parent.parent as Window;
			var p:DisplayObjectContainer = w.parent;
			var parentIndex:int = p.getChildIndex(w);
			var decoratorIndex:int = decoratorPanels.indexOf(w);
			p.removeChild(w);
			decoratorPanels.splice(decoratorIndex,1);
			decoratorPanels.splice(decoratorIndex-1,0,w);
			p.addChildAt(w,parentIndex-1);
			
			var message:XML = <msg target="ActiveBrushKit.movePointDecoratorIndex" oldIndex={decoratorIndex}  newIndex={decoratorIndex-1}/>;
			psykoSocket.sendString( message.toXMLString() );
			updateUpDownButtons();
		}
		
		private function updateUpDownButtons():void
		{
			for ( var i:int = 0; i < decoratorPanels.length; i++ )
			{
				var hb:HBox = decoratorPanels[i].titleBar.content.getChildByName("controlHolder") as HBox;
				PushButton(hb.getChildByName("up")).enabled = i > 0;
				PushButton(hb.getChildByName("down")).enabled = i < decoratorPanels.length - 1;;
			}
		}
		
		protected function onPanelClose(event:Event):void
		{
			var w:Window = event.target.parent.parent.parent.parent as Window;
			var decoratorIndex:int = decoratorPanels.indexOf(w);
			w.parent.removeChild(w);
			decoratorPanels.splice(decoratorIndex,1);
			var message:XML = <msg target="ActiveBrushKit.removePointDecorator" index={decoratorIndex}/>;
			psykoSocket.sendString( message.toXMLString() );
		}
		
		private function onInputTextChanged(event:Event):void
		{
			var slider:Object = inputToSlider[event.target];
			if ( slider is HSlider )
			{
				HSlider(slider).value = Number(InputText(event.target).text);
			} else if ( slider is HRangeSlider )
			{
				if ( InputText(event.target).name == "low" )
				{
					HRangeSlider(slider).lowValue = Number(InputText(event.target).text);
				} else {
					HRangeSlider(slider).highValue = Number(InputText(event.target).text);
				}
			} 
			
		}
		
		private function onShowInUI(event:Event):void
		{
			var cb:CheckBox = event.target as CheckBox;
			if ( uiElementToParameter[cb] )
			{
				var message:XML = <msg target="ActiveBrushKit.parameterChange"/>
				var parameter:XML = uiElementToParameter[cb];
				parameter.@showInUI= cb.selected ? "1" : "0";
				message.appendChild(parameter);
				psykoSocket.sendString( message.toXMLString() );
			}
		}
		
		private function onCheckBoxChanged(event:Event):void
		{
			var cb:CheckBox = event.target as CheckBox;
			if ( uiElementToParameter[cb] )
			{
				var message:XML = <msg target="ActiveBrushKit.parameterChange"/>;
				var parameter:XML = uiElementToParameter[cb];
				parameter.@value = cb.selected ? 1 : 0;
				message.appendChild(parameter);
				psykoSocket.sendString( message.toXMLString() );
			}
			
		}
		
		
		
		private function onRangeSliderChanged(event:Event):void
		{
			var rangeSlider:HRangeSlider = event.target as HRangeSlider;
			if ( uiElementToParameter[rangeSlider] )
			{
				var message:XML = <msg target="ActiveBrushKit.parameterChange"/>;
				var parameter:XML = uiElementToParameter[rangeSlider];
				parameter.@value1 = rangeSlider.lowValue;
				parameter.@value2 = rangeSlider.highValue;
				message.appendChild(parameter);
				psykoSocket.sendString( message.toXMLString() );
			}
			if ( sliderToInput[rangeSlider] )
			{
				var inputTexts:Array = sliderToInput[rangeSlider];
				InputText(inputTexts[0]).text = rangeSlider.lowValue.toString();
				InputText(inputTexts[1]).text = rangeSlider.highValue.toString();
			}
		}
		
		private function onSliderChanged(event:Event ):void
		{
			var slider:HSlider = event.target as HSlider;
			if ( uiElementToParameter[slider] )
			{
				var message:XML = <msg target="ActiveBrushKit.parameterChange"/>;
				var parameter:XML = uiElementToParameter[slider];
				parameter.@value = slider.value;
				message.appendChild(parameter);
				psykoSocket.sendString( message.toXMLString() );
			}
			if ( sliderToInput[slider] )
			{
				var inputText:InputText = sliderToInput[slider];
				inputText.text = slider.value.toString();
			}
		}
		
		protected function onComboboxChanged(event:Event):void
		{
			var cb:ComboBox = event.target as ComboBox;
			if ( uiElementToParameter[cb] )
			{
				var message:XML = <msg target="ActiveBrushKit.parameterChange"/>;
				var parameter:XML = uiElementToParameter[cb];
				parameter.@index = cb.selectedIndex;
				parameter.@list = cb.items.join(",");
				message.appendChild(parameter);
				psykoSocket.sendString( message.toXMLString() );
			}
			
		}
		
		
		
		private function initUI():void
		{
			stage.align = "TL";
			stage.scaleMode = "noScale";
			stage.addEventListener( Event.RESIZE, onResize );
			uiHolder = new Sprite();
			addChild(uiHolder );
			
			var vb:VBox = new VBox( uiHolder );
			
			var hb:HBox = new HBox( vb );
			messageSelector = new ComboBox(hb,0,0,"Send Message",["BrushShapeLibrary.getAvailableShapes","PointDecoratorFactory.getAvailableDecorators","ActiveBrushKit.getParameterSet"]);
			messageSelector.width = 200;
			messageSender = new PushButton(hb,0,0,"Send", sendManualMessage );
			
			logMessages = new TextArea( vb, 0,0,"" );
			logMessages.width = 300;
			logMessages.height = stage.stageHeight * 0.5;
			
			sendMessages = new TextArea( vb, 0,0,"" );
			sendMessages.width = 300;
			sendMessages.height = stage.stageHeight * 0.25;
			
			otherMessages = new TextArea( vb, 0,0,"" );
			otherMessages.width = 300;
			otherMessages.height = stage.stageHeight * 0.25;
			
			decoratorPanel = new Panel();
			decoratorPanel.width = 700;
			decoratorPanel.height = 20;
			decoratorPanel.color = 0x808080;
			var hb:HBox = new HBox( decoratorPanel,0,0);
			hb.width = 1000;
			decoratorList = new ComboBox(hb,0,0,"Point Decorators",["","","","","","",""] );
			addDecoratorBtn = new PushButton( hb,0,0,"Add", addPointDecorator );
			addDecoratorBtn.enabled = false;
			addDecoratorBtn.width = 50;
			
		 	shapeList = new ComboBox(null,0,0,"Brush Shapes",["","","","","","",""] );
			
			brushUI = new Sprite();
			addChild(brushUI);
			brushUI.x = 310;
			
			penWindow = new Window(this,1250,0,"Pen Info");
			penInfo = new TextArea(penWindow.content);
			penInfo.text = "No pen detected";
		}
		
		private function sendManualMessage( event:Event ):void
		{
			if ( messageSelector.selectedIndex > -1 )
			{
				var message:XML = <msg target={messageSelector.selectedItem}/>;
				psykoSocket.sendString( message.toXMLString() );
			}
			
		}
		
		protected function onResize(event:Event):void
		{
			if ( currentBrushPane != null )
			{
				currentBrushPane.height = stage.stageHeight -  currentBrushPane.y;
			}
			
			if ( logMessages != null )
			{
				logMessages.height = stage.stageHeight * 0.5;
				sendMessages.height = stage.stageHeight * 0.25;
				otherMessages.height = stage.stageHeight * 0.25;
			}
		}
		
		private function addPointDecorator( event:Event ):void
		{
			if ( decoratorList.selectedIndex == -1 ) return;
			var message:XML = <msg target="ActiveBrushKit.addPointDecorator" className={decoratorList.selectedItem}/>;
			psykoSocket.sendString( message.toXMLString() );
			
		}
	}
}