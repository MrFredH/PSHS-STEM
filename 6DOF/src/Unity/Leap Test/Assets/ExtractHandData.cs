using UnityEngine;
using System;
using System.IO.Ports;
using Leap;
using System.Collections.Generic;

public class ExtractHandData : MonoBehaviour {
    public String port;
    public String baud;
    public float updateInterval;
    public Vector position;
    public float pinch;
    public float rotation;

    private SerialPort sp;
    private float myDelay=0;
    public Boolean didStartup = false;
    private Controller controller;
    private Frame frame;// = controller.Frame(); // controller is a Controller object
    // Use this for initialization
    void Start()
    {
        didStartup = true;

        controller = new Controller();
        if (port == null)
        {
            port = "\\\\.\\COM16";
        }
        int j;
        if (!Int32.TryParse(baud, out j))
        {
            j = 115200;
            //baud = "115200";
        }
        sp = new SerialPort(port, j);
        if (!sp.IsOpen)
        {

            try
            {
                sp.Open();
                sp.DtrEnable = true;

                //Debug.Log ("default ReadTimeout: " + s_serial.ReadTimeout);
                //s_serial.ReadTimeout = 10;

                // clear input buffer from previous garbage
                sp.DiscardInBuffer();

            }
            catch (System.Exception e)
            {
            }
        }
    }
	// Update is called once per frame
	void Update () {

        if(didStartup!=true)
        {
            Start();
        }
        myDelay += Time.deltaTime;
        frame = controller.Frame(); // controller is a Controller object
        if (myDelay >= updateInterval)
        {
            myDelay = 0;
            if (frame.Hands.Count > 0)
            {
                List<Hand> hands = frame.Hands;
                Hand firstHand = hands[0];
                position = firstHand.PalmPosition;
                rotation = firstHand.PalmNormal.Roll;
                //rotation = firstHand.GrabAngle;
                pinch = firstHand.PinchDistance;
                if (sp.IsOpen == false)
                    didStartup = false;
                else
                    sp.WriteLine(position.x + "," + position.y + "," + position.z + "," + rotation + "," + pinch);
            }
        }
	}
    private void OnDestroy()
    {
        sp.Close();
    }
}
