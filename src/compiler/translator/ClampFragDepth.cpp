//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ClampFragDepth.cpp: Limit the value that is written to gl_FragDepth to the range [0.0, 1.0].
// The clamping is run at the very end of shader execution, and is only performed if the shader
// statically accesses gl_FragDepth.
//

#include "compiler/translator/ClampFragDepth.h"

#include "compiler/translator/FindSymbolNode.h"
#include "compiler/translator/IntermNode_util.h"
#include "compiler/translator/RunAtTheEndOfShader.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

void ClampFragDepth(TIntermBlock *root, TSymbolTable *symbolTable)
{
    // Only clamp gl_FragDepth if it's used in the shader.
    if (!FindSymbolNode(root, TString("gl_FragDepth")))
    {
        return;
    }

    TIntermSymbol *fragDepthNode = ReferenceBuiltInVariable("gl_FragDepth", *symbolTable, 300);

    TIntermTyped *minFragDepthNode = CreateZeroNode(TType(EbtFloat, EbpHigh, EvqConst));

    TConstantUnion *maxFragDepthConstant = new TConstantUnion();
    maxFragDepthConstant->setFConst(1.0);
    TIntermConstantUnion *maxFragDepthNode =
        new TIntermConstantUnion(maxFragDepthConstant, TType(EbtFloat, EbpHigh, EvqConst));

    // clamp(gl_FragDepth, 0.0, 1.0)
    TIntermSequence *clampArguments = new TIntermSequence();
    clampArguments->push_back(fragDepthNode->deepCopy());
    clampArguments->push_back(minFragDepthNode);
    clampArguments->push_back(maxFragDepthNode);
    TIntermTyped *clampedFragDepth =
        CreateBuiltInFunctionCallNode("clamp", clampArguments, *symbolTable, 100);

    // gl_FragDepth = clamp(gl_FragDepth, 0.0, 1.0)
    TIntermBinary *assignFragDepth = new TIntermBinary(EOpAssign, fragDepthNode, clampedFragDepth);

    RunAtTheEndOfShader(root, assignFragDepth, symbolTable);
}

}  // namespace sh
