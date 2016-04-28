using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using Windows.Devices.Bluetooth.Rfcomm;
using Windows.Devices.Enumeration;
using Windows.Devices.SerialCommunication;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage.Streams;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// Universal Windows Platform Project
namespace AL1VE
{
    /* IMPORTANT: in Package.appxmanifest make sure that you declare the following capabilities: 
      ... 
      <DeviceCapability Name="serialcommunication">
          <Device Id="any">
            <Function Type="name:serialPort" />
          </Device>
      </DeviceCapability>
      ... 
    */

    // MainPage
    public sealed partial class MainPage : Page
    {
        // Variables        
        private SerialDevice _serial = null;
        private ObservableCollection<DeviceInformation> _devices;
		

		// Constructor
		public MainPage()
        {
            this.InitializeComponent();
            ConnectButton.IsEnabled = false;
            _devices = new ObservableCollection<DeviceInformation>();
            Rescan();
			
        }

        // Rescan
        private async void Rescan()
        {
            try
            {
                // v1
                // var selector = SerialDevice.GetDeviceSelector();
                // var devices = await DeviceInformation.FindAllAsync(selector);
                // v2
                var selector = RfcommDeviceService.GetDeviceSelector(RfcommServiceId.SerialPort);
                var devices = await DeviceInformation.FindAllAsync(selector, null);
                // process
                for (int i = 0; i < devices.Count; i++) _devices.Add(devices[i]);             
                DeviceListSource.Source = _devices;
                ConnectButton.IsEnabled = true;
                ConnectDevices.SelectedIndex = -1;
            }
            catch (Exception ex)
            {
                MessageDialog dialog = new MessageDialog(ex.Message, "Error");
                await dialog.ShowAsync();
            }
        }

        // Connect 
        private async void Connect_Button_Click(object sender, RoutedEventArgs e)
        {
            if (ConnectDevices.SelectedIndex == -1)
            {
                MessageDialog dialog = new MessageDialog("Select a device and connect", "Info");
                await dialog.ShowAsync();
                return;
            }
            var selection = ConnectDevices.SelectedItem;
            DeviceInformation entry = (DeviceInformation)selection;
            try
            {
                _serial = await SerialDevice.FromIdAsync(entry.Id);
                ConnectButton.IsEnabled = false;

                // Settings
                _serial.WriteTimeout = TimeSpan.FromMilliseconds(1000);
                _serial.ReadTimeout = TimeSpan.FromMilliseconds(1000);
                _serial.BaudRate = System.Convert.ToUInt16(ConnectSpeed.SelectionBoxItem);
                _serial.Parity = SerialParity.None;
                _serial.StopBits = SerialStopBitCount.One;
                _serial.DataBits = 8;
                _serial.Handshake = SerialHandshake.None;

                // Info
                var message = "BaudRate \t" + _serial.BaudRate + "\n";
                message += "DataBits \t\t" + _serial.DataBits + "\n";
                message += "Parity \t\t" + _serial.Parity.ToString() + "\n";
                message += "StopBits \t\t" + _serial.StopBits + "\n";
                MessageDialog dialog = new MessageDialog(message, "Serial");
                await dialog.ShowAsync();

                // Navigate 
                this.Frame.Navigate(typeof(ControlPage), _serial);
				
            }
            catch (Exception ex)
            {
                MessageDialog dialog = new MessageDialog(ex.Message, "Error");
                await dialog.ShowAsync();
                ConnectButton.IsEnabled = true;
            }
        }
    }
}
