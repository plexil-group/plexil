<?xml version="1.0" encoding="UTF-8"?><!-- Generated by PlexiLisp --><PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator"><Node NodeType="NodeList"><NodeId>empty3</NodeId><EndCondition><Finished>
        <NodeId>two</NodeId>
      </Finished></EndCondition><PostCondition><Succeeded>
        <NodeId>two</NodeId>
      </Succeeded></PostCondition><NodeBody>
      <NodeList>
        <Node NodeType="Empty"><NodeId>one</NodeId></Node>
        <Node NodeType="Empty"><NodeId>two</NodeId><StartCondition><Finished>
              <NodeId>one</NodeId>
            </Finished></StartCondition></Node>
      </NodeList>
    </NodeBody></Node></PlexilPlan>