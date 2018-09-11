using UnityEngine;
using System;
using System.IO.Ports;
using Leap;
using System.Collections.Generic;

public class ExtractHandData : MonoBehaviour {
    private SerialPort sp;
    public String port;
    public String baud;
    Controller controller = new Controller();
    // Use this for initialization
    void Start () {
        if(port==null)
        {
            port = "\\\\.\\COM3";
        }
        int j;
        if (!Int32.TryParse(baud, out j))
        {
            j = 115200;
            //baud = "115200";
        }
        sp = new SerialPort(port, j);
    }
	
	// Update is called once per frame
	void Update () {
        Frame frame = controller.Frame(); // controller is a Controller object
        if (frame.Hands.Count > 0)
        {
            List<Hand> hands = frame.Hands;
            Hand firstHand = hands[0];
            Vector position = firstHand.StabilizedPalmPosition;
            sp.WriteLine(position.x + "," + position.y + "," + position.z + ","+firstHand.PinchDistance+",10");            
        }
	}
    private void OnDestroy()
    {
        sp.Close();
    }
}
