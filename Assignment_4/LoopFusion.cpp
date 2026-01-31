
#include <deque>
#include "llvm/Support/Compiler.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;


#include "LoopFusion.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"


using namespace llvm;

BasicBlock* getLoopEntryBlock(Loop* l) {
    
    return (l->isGuarded()) ? l->getLoopPreheader()->getUniquePredecessor() : l->getLoopPreheader();
}

BasicBlock* getLoopBody(Loop *L) {

    BranchInst *HeaderBranchInst = dyn_cast<BranchInst>(L->getHeader()->getTerminator());
    if (!HeaderBranchInst)
        return nullptr;
    
    if (L->contains(HeaderBranchInst->getSuccessor(0)))
        return HeaderBranchInst->getSuccessor(0);

    if (L->contains(HeaderBranchInst->getSuccessor(1)))
        return HeaderBranchInst->getSuccessor(1);

    return nullptr;
}

std::deque <Loop*> getLoops(LoopInfo &LI) {
    std::deque <Loop*> loops;

    for (auto *L : LI){
        loops.push_front(L);
    }

    return loops;
}

bool areAdjacent(Loop* l1, Loop* l2) {

    BasicBlock *l2EntryBlock = getLoopEntryBlock(l2);
    if (l2EntryBlock == nullptr){
        errs() << "L2 Entry is nullptr\n";
        return false;
    }

    //Se L1 è guarded
    if (l1->isGuarded()){
        BasicBlock* loopGuard = l1->getLoopPreheader()->getUniquePredecessor();
        if(loopGuard == nullptr){
            errs() << "L1 guard is nullptr\n";
            return false;
        }
        //Successori del blocco di guardia deve essere L2 entry block
        for (BasicBlock *successorBlock : successors(loopGuard)){
            if (successorBlock == l2EntryBlock)
                return true;
        }
        
        return false;
    }

    else {
        BasicBlock *l1Exit = l1->getExitBlock();
        if (l1Exit == nullptr){
            errs() << "L1 Exit is nullptr \n";
            return false;

        }
        //Sennò l'Exit Block di L1 deve essere il preheader di L2
        if (l1Exit == l2EntryBlock)
            return true;
    }

    errs() << "Loops Not Adjacent\n";
    return false;
}

bool areCFEquivalent(Loop* l1, Loop* l2, DominatorTree &DT, PostDominatorTree &PDT) {
    BasicBlock* L1Block = getLoopEntryBlock(l1);
    BasicBlock* L2PreHeader = l2->getLoopPreheader();
    // Verifica che L1Block domini L2PreHeader e che L2PreHeader post-domini L1Block
    if (DT.dominates(L1Block, L2PreHeader) && PDT.dominates(L2PreHeader, L1Block))
        return true;
    else{
        errs() << "Loops are not Control Flow equivalent\n";
        return false;
    }
        
}

bool sameTripCount(Loop* l1, Loop* l2, ScalarEvolution &SE) {
    const SCEV *l1TripCount = SE.getBackedgeTakenCount(l1);
    const SCEV *l2TripCount = SE.getBackedgeTakenCount(l2);

    //Verifica che i due trip count siano uguali
    if (l1TripCount == l2TripCount)
        return true;

    errs() << "Loops do not have the same trip count\n";
    return false;
}

bool fuseLoop(Loop* l1, Loop* l2, LoopInfo &LI) {
    // Recupera le induction variable canoniche
    PHINode *l1IndVar = l1->getCanonicalInductionVariable();
    PHINode *l2IndVar = l2->getCanonicalInductionVariable();
    // Verifica che entrambe le induction variable siano valide
    if (!l1IndVar || !l2IndVar) {
        errs() << "One of the loops does not have a canonical induction variable.\n";
        return false;
    }

    // Sostituisci tutti gli usi della induction variable del secondo loop con quella del primo
    l2IndVar->replaceAllUsesWith(l1IndVar);

    // Blocchi chiave
    BasicBlock *l1Header = l1->getHeader();
    BasicBlock *l1Latch  = l1->getLoopLatch();
    BasicBlock *l2Header = l2->getHeader();
    BasicBlock *l2Latch  = l2->getLoopLatch();
    BasicBlock *l2Exit   = l2->getExitBlock();
    BasicBlock *l2PreHeader = getLoopEntryBlock(l2);
    BasicBlock *l2Body = getLoopBody(l2);

    if (!l1Header || !l1Latch || !l2Header || !l2Latch || !l2Exit || !l2PreHeader || !l2Body) {
        errs() << "Some basic blocks required for fusion are missing.\n";
        return false;
    }

    // Collega il latch del primo loop al corpo del secondo loop
    for (auto *pred : predecessors(l1Latch)) {
        if (pred != l1Header) // evita di modificare il backedge principale
            pred->getTerminator()->replaceSuccessorWith(l1Latch, l2Body);
    }

    // Collega il latch del secondo loop al latch del primo loop (chiusura ciclo fuso)
    for (auto *pred : predecessors(l2Latch)) {
        if (pred != l2Header)
            pred->getTerminator()->replaceSuccessorWith(l2Latch, l1Latch);
    }

    // Collega l'header del primo loop direttamente all'exit del secondo
    l1Header->getTerminator()->replaceSuccessorWith(l2PreHeader, l2Exit);

    // Collega l'header del secondo loop al suo corpo
    l2Header->getTerminator()->replaceSuccessorWith(l2Body, l2Latch);

    // Sposta i blocchi del secondo loop (tranne header e latch) nel primo loop
    std::vector<BasicBlock*> blocksToTransfer;
    for (auto *block : l2->blocks()) {
        if (block != l2Header && block != l2Latch) {
            blocksToTransfer.push_back(block);
        }
    }
    for (auto *block : blocksToTransfer) {
        l2->removeBlockFromLoop(block);
        l1->addBasicBlockToLoop(block, LI);
    }

    return true;
}

// Verifica l'assenza di dipendenze di distanze negative
bool areNotNegativeDistanceDependent(Loop* l1, Loop* l2, DependenceInfo &DI, ScalarEvolution &SE) {
    for (auto *BBLoop1 : l1->blocks()) {
        for (auto &InstLoop1 : *BBLoop1) {
            for (auto *BBLoop2 : l2->blocks()) {
                for (auto &InstLoop2 : *BBLoop2) {
                    //Per ogni istruzione in L1 e L2, verifica se esiste una dipendenza
                    if (auto Dep = DI.depends(&InstLoop1, &InstLoop2, true)) {
                        if (Dep) {
                            //Se esiste una dipendenza, controlla i livelli e le distanze
                            unsigned Levels = Dep->getLevels();
                            for (unsigned i = 1; i <= Levels; ++i) {
                                const SCEV *distSCEV = Dep->getDistance(i);
                                if (distSCEV) {
                                    
                                    if (const SCEVConstant *C = dyn_cast<SCEVConstant>(distSCEV)) {
                                        int64_t dist = C->getAPInt().getSExtValue();
                                        if (dist < 0) {
                                            errs() << "Negative distance dependence found between:\n";
                                            errs() << "  Instruction in L1: " << InstLoop1 << "\n";
                                            errs() << "  Instruction in L2: " << InstLoop2 << "\n";
                                            return false;
                                        }
                                    }
                                    // Se non è costante, non possiamo garantire l'assenza di dipendenza negativa
                                    //Puo essere un espressione simbolica
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    outs() << "No negative distance dependence found\n";
    return true;
}

PreservedAnalyses LoopFusionPass::run(Function &F, FunctionAnalysisManager &AM) {

    std::deque <Loop*> loops;
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);

    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);

    ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
    DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);

    bool fusionOccurred;

    do {
        outs() << "\n-----------------------\n";
        fusionOccurred = false;
        loops = getLoops(LI);
        
        // Controlla che ci siano almeno due loop da fondere
        if (loops.size() < 2){
            outs() << "Insufficient number of loops\n";
            return PreservedAnalyses::all();
        }

        for (int i=0; i<loops.size()-1; i++){
            Loop* L1 = loops[i];
            Loop* L2 = loops[i+1];

            // Verifica che i loop non siano nulli
            if (!L1 || !L2) {
                errs() << "One of the loops is null.\n";
                return PreservedAnalyses::all();
            }

            outs() << "ITERATION NUMBER : " << i << "\n";
            outs() << "*** LOOP " << i << " ***\n\n";
            for (auto *BB : L1->blocks()) {
                BB->print(outs());
                outs() << "\n";
            }

            outs() << "\n\n*** LOOP " << i+1 << " ***\n\n";
            for (auto *BB : L2->blocks()) {
                BB->print(outs());
                outs() << "\n";
            }

            if(areCFEquivalent(L1, L2, DT, PDT) && areAdjacent(L1, L2) && sameTripCount(L1, L2, SE) && areNotNegativeDistanceDependent(L1, L2, DI, SE)){
                outs() << "Loop " << i << " and " << i+1 << " are Loop Fusion Candidates\n";
                
                
                if(fuseLoop(L1, L2, LI)){
                    LI.erase(L2);
                    outs() << "Fused Loop " << i << " and " << i+1 << "\n"; 
                    EliminateUnreachableBlocks(F);

                    fusionOccurred = true;
                    break;
                }
                else{
                    outs() << "Unable to fuse Loop " << i << " and " << i+1 << "\n"; 
                }              
            }
            else {
                outs() << "Loop " << i << " and " << i+1 << " are NOT Loop Fusion Candidates\n";
            }  
        }
    }
    while(fusionOccurred);
    
    
    return PreservedAnalyses::all();
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "LoopFusion", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                     ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "LoopFusion") {
                        FPM.addPass(LoopFusionPass());
                        return true;
                    }
                    return false;
                });
        }
    };
}