open Docker;
open Dockerode;
open Sandboxnet;

let nodePort = Config.getExn("tezosNode.networks.<network>.rpcPort");
let nodeContainerName = Config.getExn("tezosNode.networks.<network>.containerName");
let nodeDataDir = cwd ++ "/" ++ Config.getExn("tezosNode.networks.<network>.dataDir");
let dockerNetworkName = Config.getExn("dockerNetworkName");
let dockerImage = Config.getExn("tezosNode.networks.<network>.dockerImage");


let ports = Js.Dict.empty();
Js.Dict.set(ports, nodePort ++ "/tcp", [|
    portBinding(~hostPort=nodePort)
|])
let containerHostConfig: hostConfiguration = hostConfiguration(
    ~volumeBindings = Some([|cwd ++ ":" ++ cwd|]),
    ~portBindings = Some(ports),
    ~networkMode = Some(dockerNetworkName)
);

let containerOptions = createContainerOptions(
    ~image=Some(dockerImage),
    ~cmd= Some([|
        "run",
        "--sandbox=" ++ nodeInitDestinationPath ++ "/sandbox.json",
        "--data-dir=" ++ nodeDataDir ,
        "--no-bootstrap-peers",
        "--private-mode",
        "--peer=localhost:19732",
        "--rpc-addr=:" ++ nodePort ++ ""
    |]),
    ~hostConfig= Some(containerHostConfig),
    ~name=Some(nodeContainerName),
    ~tty=Some(false)
);

let start = () => {

    Docker.start(containerOptions)
        |> Js.Promise.then_((container) => {
            Js.Promise.resolve(container);
        })
};
let stop = () => Docker.stop(nodeContainerName)