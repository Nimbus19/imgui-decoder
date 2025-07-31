using System;
using System.Collections;
using System.Runtime.InteropServices;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class NewBehaviourScript : MonoBehaviour
{
    [SerializeField] private Button createBtn;
    [SerializeField] private Button decodeBtn;
    [SerializeField] private Button renderBtn;
    [SerializeField] private Button distroyBtn;
    [SerializeField] private RawImage rawImage;
    [SerializeField] private TMP_InputField inputField;

    IntPtr decoderPtr = IntPtr.Zero;
    IntPtr d3dDevice = IntPtr.Zero;
    IntPtr d3dContext = IntPtr.Zero;
    IntPtr nativeTexPtr = IntPtr.Zero;
    Texture2D texture;

    void Start()
    {
        createBtn.onClick.AddListener(() =>
        {
            d3dDevice = GetD3D11Device();
            d3dContext = GetD3D11Context();
            Log($"D3D Device: {d3dDevice}, D3D Context: {d3dContext}");

            decoderPtr = CreateWIN(d3dDevice, d3dContext);
            if (decoderPtr == IntPtr.Zero)
            {
                LogError("Create Decoder Failed");
            }
            else
            {
                Log("Create Decoder Succeeded");
            }
        });

        decodeBtn.onClick.AddListener(() =>
        {
            bool result = DecodeWIN(decoderPtr); // 這裡需要傳入實際的解碼器指針
            if (result == false)
            {
                LogError("Decode Failed");
            }
            else
            {
                Log("Decode Succeeded");
                while (result)
                {
                    result = DecodeWIN(decoderPtr);
                    if (result)
                        Log("Decoding...");
                }
            }
        });

        renderBtn.onClick.AddListener(() =>
        {
            bool result = RenderWIN(decoderPtr); // 這裡需要傳入實際的解碼器指針
            if (result == false)
            {
                LogError("Render Failed");
            }
            else
            {
                Log("Render Succeeded");
                ShowTexture();
            }
        });

        distroyBtn.onClick.AddListener(() =>
        {
            IntPtr decoderPtr = IntPtr.Zero; // 這裡需要傳入實際的解碼器指針
            DestroyWIN(decoderPtr);
            Log("Destroy Decoder Succeeded");
        });
    }

    void ShowTexture()
    {
        if (nativeTexPtr == IntPtr.Zero)
        {
            nativeTexPtr = GetTextureWIN(decoderPtr); // 這裡需要傳入實際的解碼器指針
            if (nativeTexPtr == IntPtr.Zero)
            {
                LogError("Get Native Texture Failed");
                return;
            }

            texture = Texture2D.CreateExternalTexture(1280 * 2, 720, TextureFormat.Alpha8, false, false, nativeTexPtr);
            if (texture == null)
            {
                LogError("Create External Texture Failed");
                return;
            }

            rawImage.texture = texture;
            //rawImage.SetNativeSize();
            Log("Show Texture Succeeded");
        }
    }

    private void Log(string message)
    {
        Debug.Log(message);
        inputField.text = message;
    }

    private void LogError(string message)
    {
        Debug.LogError(message);
        inputField.text = message;
    }


#if UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
    const string DllName = "DecoderDemo";
#elif UNITY_EDITOR_OSX || UNITY_STANDALONE_OSX
    const string DllName = "libDecoderDemo";
#elif UNITY_IOS
    const string DllName = "__Internal";
#elif UNITY_ANDROID
    const string DllName = "DecoderDemo";
#endif

#if UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
    [DllImport(DllName)] private static extern IntPtr CreateWIN(IntPtr d3d_device, IntPtr d3d_context);
    [DllImport(DllName)] private static extern bool DecodeWIN(IntPtr decoder);
    [DllImport(DllName)] private static extern bool RenderWIN(IntPtr decoder);
    [DllImport(DllName)] private static extern void DestroyWIN(IntPtr decoder);
    [DllImport(DllName)] private static extern IntPtr GetTextureWIN(IntPtr decoder);
    [DllImport(DllName)] private static extern IntPtr GetD3D11Device();
    [DllImport(DllName)] private static extern IntPtr GetD3D11Context();

#elif UNITY_EDITOR_OSX || UNITY_STANDALONE_OSX || UNITY_IOS
   
#elif UNITY_ANDROID

#endif
}
