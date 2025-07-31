// ------------------------------------------------------------
// YUV Texture Shader
// Author: Wythe
// ------------------------------------------------------------
Shader "Unlit/Waninkit/YUVTexture"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
    }
    SubShader
    {
        // No depth test
        ZWrite Off ZTest Always
        Blend SrcAlpha OneMinusSrcAlpha
        Tags { "Queue" = "Transparent" }

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #include "UnityCG.cginc"

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float4 vertex : SV_POSITION;
            };

            v2f vert (appdata v)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.uv = float2(v.uv.x, 1.0 - v.uv.y);
                return o;
            }

            sampler2D _MainTex;
            
            fixed4 frag (v2f i) : SV_Target
            {
                return tex2D(_MainTex, i.uv);
            }
            ENDCG
        }
    }
}
