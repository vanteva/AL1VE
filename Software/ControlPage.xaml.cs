using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.SerialCommunication;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage.Streams;
using Windows.UI.Core;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace AL1VE
{
	// Command Page
	public sealed partial class ControlPage : Page
	{
		// Private variables 
		private SerialDevice _serial = null;
		private DataReader dataReaderObject = null;
		private CancellationTokenSource ReadCancellationTokenSource;

		// Left & Right
		private char L = '^';
		private char R = '^';


		// Constructor
		public ControlPage()
		{
			this.InitializeComponent();
			
		}

		// OnLoad
		protected override void OnNavigatedTo(NavigationEventArgs e)
		{
			var item = (SerialDevice)e.Parameter;
			_serial = item;
			// Cancelation Token Magic!
			ReadCancellationTokenSource = new CancellationTokenSource();
			Listen();
		}

		// Left
		private void Left_Value_Changed(object sender, RangeBaseValueChangedEventArgs e)
		{
			double temp = Left.Value;
			if (temp > 0) R = (char)(temp + 96);
			if (temp < 0) R = (char)(Math.Abs(temp) + 64);
			if (temp == 0) R = '^';
			string cmd = L.ToString() + R.ToString() + "\r";
			SendCommand(cmd);
		}

		// Right
		private void Right_Value_Changed(object sender, RangeBaseValueChangedEventArgs e)
		{
			double temp = Right.Value;
			if (temp > 0) L = (char)(temp + 96);
			if (temp < 0) L = (char)(Math.Abs(temp) + 64);
			if (temp == 0) L = '^';
			string cmd = L.ToString() + R.ToString() + "\r";
			SendCommand(cmd);
		}

		// Send Button
		private void Stop_Button_Click(object sender, RoutedEventArgs e)
		{
			Left.Value = Right.Value = 0; // Set Sliders to Zero
			string cmd = "^^\r"; // Send Full Stop
			SendCommand(cmd);
		}

		// Asynchronous Task to Send Command  
		private async void SendCommand(String cmd)
		{
			try
			{
				IBuffer buffer = Encoding.UTF8.GetBytes(cmd).AsBuffer();
				await _serial.OutputStream.WriteAsync(buffer);
				speedInfo.Text = (Left.Value +" "+ Right.Value);
			}
			catch (Exception ex)
			{
				MessageDialog dialog = new MessageDialog(ex.Message, "Error");
				await dialog.ShowAsync();
			}
		}

		// Listen from Seril Port
		private async void Listen()
		{
			while (true)
			{
				try
				{
					if (_serial != null)
					{
						dataReaderObject = new DataReader(_serial.InputStream);

						// keep reading the serial input
						while (true)
						{
							await ReadAsync(ReadCancellationTokenSource.Token);
						}
					}
				}
				catch (Exception ex)
				{
					if (ex.GetType().Name == "TaskCanceledException")
					{
						// status.Text = "Reading task was cancelled, closing device and cleaning up";
						CloseDevice();
					}
					else
					{
						// status.Text = ex.Message;
					}
				}
				finally
				{
					// Cleanup once complete
					if (dataReaderObject != null)
					{
						dataReaderObject.DetachStream();
						dataReaderObject = null;
					}
				}
			}
		}

        // Data Collection
        public List<Temp> _temp = new List<Temp>();
		public List<Temp1> _pres = new List<Temp1>();
		public List<Temp> _hum = new List<Temp>();

		// Read Fro Serial Port
		private async Task ReadAsync(CancellationToken cancellationToken)
		{
			Task<UInt32> loadAsyncTask;
			uint ReadBufferLength = 32;

			// Set InputStreamOptions to complete the asynchronous read operation when one or more bytes is available
			dataReaderObject.InputStreamOptions = InputStreamOptions.Partial;

			// Create a task object to wait for data on the serialPort.InputStream
			loadAsyncTask = dataReaderObject.LoadAsync(ReadBufferLength).AsTask();

			// Launch the task and wait
			UInt32 bytesRead = await loadAsyncTask;
			if (bytesRead > 0)
			{
				string data = dataReaderObject.ReadString(bytesRead);
				string[] splitted = data.Split(' ');
				tempr.Text = data;
				t1.Text = "T:" + splitted[6]+"C";
				h1.Text = "H:" + splitted[5]+"%";
				double voltage = double.Parse(splitted[1]) * 0.0048828125;
				double distance = 2.3533f * Math.Pow(voltage, 6.0) - 25.815f * Math.Pow(voltage, 5.0) + 113.85f * Math.Pow(voltage, 4.0)
								- 259.79f * Math.Pow(voltage, 3.0) + 329.53f * Math.Pow(voltage, 2.0) - 232.68f * voltage + 85.396f;
				d1.Text = "D:" + distance+"cm";
				p1.Text = "Pa:" + splitted[3];
                // new
                _temp.Add(new Temp() { Value = float.Parse(splitted[6]) });
				_pres.Add(new Temp1() { Value1 = float.Parse(splitted[3])});
            }
			//tempr.Text = dataReaderObject.ReadString(bytesRead);

		}

		// Close Serial Port
		private void CloseDevice()
		{
			if (_serial != null)
			{
				_serial.Dispose();
			}
			_serial = null;
		}

        // Go to Temp Chart
		private void InfoButton_Click(object sender, RoutedEventArgs e)
		{
			this.Frame.Navigate(typeof(TempChart), _temp);
		}
	}
}
