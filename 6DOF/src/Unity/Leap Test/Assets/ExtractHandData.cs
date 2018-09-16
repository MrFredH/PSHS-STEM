using UnityEngine;
using System;
using System.IO.Ports;
using Leap;
using System.Collections.Generic;
//using System.Threading;

public class ExtractHandData : MonoBehaviour
{
    public String port;
    public String baud;
    public float updateInterval;
    public Vector position;
    public float pinch;
    public float rotation;

    //    private static Boolean _continue = true;
    private SerialPort sp;
    private float myDelay = 0;
    public Boolean didStartup = false;
    private Controller controller;
    private Frame frame;// = controller.Frame(); // controller is a Controller object
    // Use this for initialization
    void Start()
    {
        myDelay = Time.time;
        didStartup = true;
        //Thread readThread = new Thread(Read);
        controller = new Controller();
        int j;
        if (!Int32.TryParse(baud, out j))
        {
            j = 115200;
            //baud = "115200";
        }
        if (port == null || port.Trim().Length <= 2)
        {
            Debug.Log("Port is null or blank");
            //            Console.WriteLine("Port is null or blank");
            String[] portNames = System.IO.Ports.SerialPort.GetPortNames();
            DateTime now = DateTime.Now;
            foreach (string str in portNames)
            {
                String s = "\\\\.\\" + str;
                //Console.WriteLine(s);
                Debug.Log(s);
                //port = s;
                sp = new SerialPort(s, j);
                
                //                if (!sp.IsOpen)
                //                {
                now = DateTime.Now;
                try
                {
                    while (!sp.IsOpen&&DateTime.Now.Subtract(now).TotalMilliseconds < 500)
                    {
                    }
                    //if(!sp.IsOpen)
                    //{
                    //    sp.Open();
                    //}
                }
                catch (Exception e)
                {
                    Debug.Log("Open:" + e.ToString());
                }
                if (sp.IsOpen)
                {
                    try
                    {
                        sp.DtrEnable = true;
                    }
                    catch (Exception e)
                    {
                        Debug.Log("DTR:" + e.ToString());
                    }

                    //Debug.Log ("default ReadTimeout: " + s_serial.ReadTimeout);
                    //s_serial.ReadTimeout = 10;

                    // clear input buffer from previous garbage
                    try
                    {
                        sp.DiscardInBuffer();
                    }
                    catch (Exception e)
                    {
                        Debug.Log("Disc:" + e.ToString());
                    }
                    try
                    {
                        sp.ReadTimeout = 500;
                        sp.WriteTimeout = 500;
                    }
                    catch (Exception e)
                    {
                        Debug.Log("TOuts:" + e.ToString());

                    }
                    try
                    {
                        sp.WriteLine("6DOF?");
                    }
                    catch (System.Exception e)
                    {
                        Debug.Log("Write:" + e.ToString());
                    }
                    //                        sp.BaseStream.Flush();
                    //Console.WriteLine(s);
                    String reply = "";
                    try
                    {

                        //Debug.Log(s);
                        now = DateTime.Now;
                        while (DateTime.Now.Subtract(now).TotalMilliseconds < 500)
                        {
                            if (sp.BytesToRead > 0)
                            {
                                reply += sp.ReadExisting();
                            }
                            if (reply.Contains("\n") || reply.Contains("\r"))
                            {
                                break;
                            }
                        }
                    }
                    catch (System.Exception e)
                    {
                        if (e != null)
                        {
                            Debug.Log(e.ToString());
                        }
                    }
                    Debug.Log(reply);
                    if (reply.Contains("6DOF=Yes"))
                    {
                        port = (String)s.Clone();
                    }
                }
            }
        }
        //port = "\\\\.\\COM16";
        else
        {
            //            sp = new SerialPort(port, j, Parity.None, 8, StopBits.One);
            //sp.Handshake = Handshake.RequestToSendXOnXOff;
            sp = new SerialPort(port, j);
        }
        if (!sp.IsOpen)
        {

            try
            {
                sp.Open();
                //sp.DtrEnable = true;

                // clear input buffer from previous garbage
                sp.DiscardInBuffer();
                //sp.DiscardOutBuffer();
            }
            catch (Exception e)
            {
                if (e != null)
                {
                    Debug.Log(e.ToString());
                }
            }
        }
    }
    
    // Update is called once per frame
    void Update()
    {
        if (didStartup != true)
        {
            Start();
        }
        
        if (Time.time >= myDelay)
        {
            frame = controller.Frame(); // controller is a Controller object
            myDelay = Time.time + updateInterval;
            if (frame.Hands.Count > 0)
            {
                List<Hand> hands = frame.Hands;
                Hand firstHand = hands[0];
                position = firstHand.PalmPosition;
                rotation = firstHand.PalmNormal.Roll;
                //rotation = firstHand.GrabAngle;
                pinch = firstHand.PinchDistance;
                if (sp.IsOpen == false)
                {
                    //didStartup = false;
                }
                else
                {
                    // Check if comms buffer is half full, abandon further writes till it clears.
//                    if(sp.BytesToWrite<=200)

            //        if (sp.CtsHolding == true)
                    {
                        sp.WriteLine(position.x + "," + position.y + "," + position.z + "," + rotation + "," + pinch);
                        sp.DiscardInBuffer();
                        sp.BaseStream.Flush();
                    }
                }
            }
        }
    }
    private void OnApplicationQuit()
    {
        if (sp != null)
        {
            sp.Close();
            sp = null;
        }
    }
    private void OnDestroy()
    {
        //_continue = false;
        if (sp != null)
        {
            sp.Close();
            sp = null;
        }
    }
}
