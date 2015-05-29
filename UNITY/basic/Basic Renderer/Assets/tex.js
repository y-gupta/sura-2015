#pragma strict
//using UnityEditor;
function Start () {
	var mesh : Mesh = GetComponent.<MeshFilter>().mesh;
	var vertices : Vector3[]  = mesh.vertices;
	var uvs : Vector2[] = new Vector2[vertices.Length];

	for (var i = 0 ; i < uvs.Length; i++)
	{
		var x=vertices[i].x;
		var y=vertices[i].z;
		x=x-Mathf.Floor(x);
		y=y-Mathf.Floor(y);
		uvs[i] = Vector2 (x,y);
	}
	mesh.uv = uvs;
	Debug.Log(mesh.uv[5]);
	Unwrapping.GenerateSecondaryUVSet(mesh);
}

function Update () {

}