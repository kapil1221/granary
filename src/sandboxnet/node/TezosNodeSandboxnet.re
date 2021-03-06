open Docker;
open Dockerode;
open Data;
open Log;

let nodePort = Config.getExn("tezosNode.networks.<network>.rpcPort");
let nodeContainerName = Config.getExn("tezosNode.networks.<network>.containerName");
let nodeDataDir = cwd ++ "/" ++ Config.getExn("tezosNode.networks.<network>.dataDir");
let dockerNetworkName = Config.getExn("dockerNetworkName");
let dockerImage = Config.getExn("tezosNode.networks.<network>.dockerImage");

let dirname: string = [%bs.raw {|__dirname|}];
let nodeBaseInitFilePath: string = dirname ++ "/init";
let nodeInitDestinationPath = Config.getExn("data.networks.<network>.nodeFolder");

let nodeInitFiles = [
    "config.json",
    "identity.json",
    "sandbox.json",
    "version.json"
];
let init = () => {
    log(":point_right:  Setting up sandboxnet node");
    Data.init(nodeBaseInitFilePath, nodeInitDestinationPath, nodeInitFiles);
    Docker.pull(dockerImage) |> ignore;
}
let clean = () => Data.clean(nodeInitDestinationPath);


let ports = Js.Dict.empty();
Js.Dict.set(ports, nodePort ++ "/tcp", [|
    portBinding(~hostPort=nodePort)
|]);

let exposedPorts = Js.Dict.empty();
Js.Dict.set(exposedPorts, nodePort ++ "/tcp", [%bs.raw {|{}|}]);

let containerHostConfig: hostConfiguration = hostConfiguration(
    ~volumeBindings = Some([|cwd ++ ":" ++ cwd|]),
    ~portBindings = Some(ports),
    ~networkMode = Some(dockerNetworkName)
);

let containerOptions = createContainerOptions(
    ~image=Some(dockerImage),
    ~cmd= Some([|
        "run",
        "--sandbox=" ++ nodeDataDir ++ "/sandbox.json",
        "--data-dir=" ++ nodeDataDir ,
        "--no-bootstrap-peers",
        "--private-mode",
        "--peer=localhost:19732",
        "--rpc-addr=:" ++ nodePort ++ ""
    |]),
    ~hostConfig= Some(containerHostConfig),
    ~name=Some(nodeContainerName),
    ~tty=Some(true),
    ~exposedPorts=Some(exposedPorts)
);

let start = () => {
    Docker.run(containerOptions);
};
let stop = () => Docker.stop(nodeContainerName);

let clean = () => Data.clean(nodeInitDestinationPath);