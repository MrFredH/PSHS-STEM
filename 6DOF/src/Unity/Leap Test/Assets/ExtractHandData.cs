//using System.Collections;
//using System.Collections.Generic;
using UnityEngine;
//using System;
//using System.Collections;
//using System.Collections.Generic;
using System;
using System.IO.Ports;
using Leap;


public class ExtractHandData : MonoBehaviour {
    private SerialPort sp;
    public String port;
    public String baud;
    // Use this for initialization
    void Start () {
        if(port==null)
        {
            port = "\\\\.\\COM3";
        }
        if(baud==null)
        {
            baud = "115200";
        }
        sp = new SerialPort(port, baud);
    }
	
	// Update is called once per frame
	void Update () {
        Frame frame = controller.Frame(); // controller is a Controller object
        if (frame.Hands.Count > 0)
        {
            List<Hand> hands = frame.Hands;
            Hand firstHand = hands[0];
            Vector position = firstHand.PalmPosition;
            
        }
        
	}
}
