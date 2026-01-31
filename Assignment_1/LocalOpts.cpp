#include "LocalOpts.h" 
#include "llvm/IR/Module.h"       
#include "llvm/IR/Function.h"    
#include "llvm/IR/BasicBlock.h"   
#include "llvm/IR/Instructions.h" 
#include "llvm/IR/InstrTypes.h"   
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

//Obtiene la costante e il parametro dall'istruzione binaria
bool getConstantFromInstruction(Instruction &inst, ConstantInt *&C, Value *&Param){
    if (auto *constant = dyn_cast<ConstantInt>(inst.getOperand(0))) {
        C = constant;
        Param = inst.getOperand(1);
    }
    else if (auto *constant = dyn_cast<ConstantInt>(inst.getOperand(1))) {
        C = constant;
        Param = inst.getOperand(0);
    }
    else {
        return false;
    }
    return true;
}

bool algebraicIdentity(BasicBlock &B) {
    unsigned int instOpcode;
    ConstantInt *C;
    Value *Param;

    for (auto inst = B.begin(); inst != B.end(); ) {
        Instruction &Inst = *inst++;
        instOpcode = Inst.getOpcode();

        switch(instOpcode) {
            // Addizione
            case Instruction::Add:
                if (!(getConstantFromInstruction(Inst, C, Param)))
                    break;
                // x + 0 = x   0 + x = x
                if (C->isZero()) {
                    Inst.replaceAllUsesWith(Param);
                }
                break;

            // Sottrazione
            case Instruction::Sub:
                if (!(getConstantFromInstruction(Inst, C, Param)))
                    break;
                // x - 0 = x
                if (Inst.getOperand(1) == C && C->isZero()) {
                    Inst.replaceAllUsesWith(Inst.getOperand(0));
                }
                // 0 - x = -x
                else if (Inst.getOperand(0) == C && C->isZero()) {
                    Instruction *neg = BinaryOperator::CreateNeg(Inst.getOperand(1));
                    neg->insertAfter(&Inst);
                    Inst.replaceAllUsesWith(neg);
                }
                break;

            // Moltiplicazione
            case Instruction::Mul:
                if (!(getConstantFromInstruction(Inst, C, Param)))
                    break;
                // x * 1 = x  1 * x = x
                if (C->equalsInt(1)) {
                    Inst.replaceAllUsesWith(Param);
                }
                // x * -1 = -x  -1 * x = -x
                else if (C->equalsInt(-1)) {
                    Instruction *neg = BinaryOperator::CreateNeg(Param);
                    neg->insertAfter(&Inst);
                    Inst.replaceAllUsesWith(neg);
                }
                break;

            // Divisione
            case Instruction::SDiv: {
                Value *op0 = Inst.getOperand(0);
                Value *op1 = Inst.getOperand(1);
                auto *constOp = dyn_cast<ConstantInt>(op1);

                if (!constOp)
                    break;

                C = constOp;
                Param = op0;
                // x / 1 = x
                if (C->equalsInt(1)) {
                    Inst.replaceAllUsesWith(Param);
                }
                // x / -1 = -x
                else if (C->equalsInt(-1)) {
                    Instruction *neg = BinaryOperator::CreateNeg(Param);
                    neg->insertAfter(&Inst);
                    Inst.replaceAllUsesWith(neg);
                }
                break;
            }

            default:
                break;
        }
    }
    return true;
}

bool strengthReduction(BasicBlock &B) {
    unsigned int instOpcode;
    ConstantInt *C;
    Value *Param;

    for (auto inst = B.begin(); inst != B.end(); ++inst) {
        Instruction &Inst = *inst;
        instOpcode = Inst.getOpcode();

        switch(instOpcode) {
            case Instruction::Mul:
                if (!(getConstantFromInstruction(Inst, C, Param)))
                    break;

				//Costante è potenza di 2
                if (C->getValue().isPowerOf2()) {
                    Constant *shiftCount = ConstantInt::get(C->getType(), C->getValue().exactLogBase2());
                    Instruction *shift_left = BinaryOperator::Create(BinaryOperator::Shl, Param, shiftCount);
                    shift_left->insertAfter(&Inst);
                    Inst.replaceAllUsesWith(shift_left);
                }
				
				//Costante - 1 è potenza di 2
                else if ((C->getValue() - 1).isPowerOf2()) {
                    Constant *shiftCount = ConstantInt::get(C->getType(), (C->getValue() - 1).exactLogBase2());
                    Instruction *shift_left = BinaryOperator::Create(BinaryOperator::Shl, Param, shiftCount);
                    shift_left->insertAfter(&Inst);

                    Instruction *new_add = BinaryOperator::Create(BinaryOperator::Add, shift_left, Param);
                    new_add->insertAfter(shift_left);
                    Inst.replaceAllUsesWith(new_add);
                }
				
				//Costante + 1 è potenza di 2
				else if ((C->getValue() + 1).isPowerOf2()) {
					Constant *shiftCount = ConstantInt::get(C->getType(), (C->getValue() + 1).exactLogBase2());
					Instruction *shift_left = BinaryOperator::Create(BinaryOperator::Shl, Param, shiftCount);
					shift_left->insertAfter(&Inst);

					Instruction *new_sub = BinaryOperator::Create(BinaryOperator::Sub, shift_left, Param);
					new_sub->insertAfter(shift_left);
					Inst.replaceAllUsesWith(new_sub);
				}
                break;

            case Instruction::SDiv:
                if (!(getConstantFromInstruction(Inst, C, Param)))
                    break;

				//Costante è potenza di 2
                if (C->getValue().isPowerOf2()) {
                    Constant *shiftCount = ConstantInt::get(C->getType(), C->getValue().exactLogBase2());
                    Instruction *shift_right = BinaryOperator::Create(BinaryOperator::AShr, Param, shiftCount);
                    shift_right->insertAfter(&Inst);
                    Inst.replaceAllUsesWith(shift_right);
                }
				//Costante - 1 è potenza di 2
                else if ((C->getValue() - 1).isPowerOf2()) {
                    Constant *shiftCount = ConstantInt::get(C->getType(), (C->getValue() - 1).exactLogBase2());
                    Instruction *shift_right = BinaryOperator::Create(BinaryOperator::AShr, Param, shiftCount);
                    shift_right->insertAfter(&Inst);

                    Instruction *new_add = BinaryOperator::Create(BinaryOperator::Add, shift_right, Param);
                    new_add->insertAfter(shift_right);
                    Inst.replaceAllUsesWith(new_add);
                }
				//Costante + 1 è potenza di 2
				else if ((C->getValue() + 1).isPowerOf2()) {
					Constant *shiftCount = ConstantInt::get(C->getType(), (C->getValue() + 1).exactLogBase2());
					Instruction *shift_right = BinaryOperator::Create(BinaryOperator::AShr, Param, shiftCount);
					shift_right->insertAfter(&Inst);

					Instruction *new_sub = BinaryOperator::Create(BinaryOperator::Sub, shift_right, Param);
					new_sub->insertAfter(shift_right);
					Inst.replaceAllUsesWith(new_sub);
				}
                break;

            default:
                break;
        }
    }
    return true;
}

bool multiInstructionOptimization(BasicBlock &B){
    for (auto inst = B.begin(); inst != B.end(); ++inst){
        Instruction &Inst = *inst;
        unsigned int instructionOpcode = Inst.getOpcode();

        // Considera solo addizioni e sottrazioni
        if (!(instructionOpcode == Instruction::Add || instructionOpcode == Instruction::Sub))
            continue;

        ConstantInt *C;
        Value *Param;
        if (!(getConstantFromInstruction(Inst, C, Param)))
            continue;

        // Calcola l'operazione opposta
        Instruction::BinaryOps oppositeOpCode = (instructionOpcode == Instruction::Add) ? Instruction::Sub : Instruction::Add;

        for (auto userInst = Inst.user_begin(); userInst != Inst.user_end(); ++userInst) {
            if (auto *casted = dyn_cast<Instruction>(*userInst)){
                Instruction &user_instruction = *casted;

                // Se l'istruzione utente è dell'operazione opposta
                if (user_instruction.getOpcode() == oppositeOpCode){
                    ConstantInt *userC;
                    Value *userParam;
                    if (!(getConstantFromInstruction(user_instruction, userC, userParam)))
                        continue;

                    // Pattern 1: a = b + c; d = a - c;  oppure  a = b - c; d = a + c;
                    if (userParam == &Inst && C->getValue() == userC->getValue()) {
                        user_instruction.replaceAllUsesWith(Param);
                    }
                }
            }
        }
    }
    return true;
}


bool deadCodeElimination(BasicBlock &B){

	auto inst = B.begin();

	while(inst != B.end()){
		Instruction &Inst = *inst;

		if ( (Inst.hasNUses(0)) && (Inst.isBinaryOp()) ){
			inst = Inst.eraseFromParent();
		}
		else{
			inst++;
		}
	}
	return true;
}


bool runOnFunction(Function &F) {
  	bool Transformed = false;

	//Itera sui BasicBlock della funzione
	for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
		if (algebraicIdentity(*Iter)) {
			Transformed = true;
		}
		if (strengthReduction(*Iter)) {
			Transformed = true;
		}
		if (multiInstructionOptimization(*Iter)) {
			Transformed = true;
		}
		if (deadCodeElimination(*Iter)){
			Transformed = true;
		}
	}

	return Transformed;
}



PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {

	//Itera sulle funzioni del modulo
	for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter)
		if (runOnFunction(*Fiter))
			return PreservedAnalyses::none();
	
	return PreservedAnalyses::all();
}


extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "LocalOpts", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, ModulePassManager &MPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "local-opts") {
            MPM.addPass(LocalOpts());
            return true;
          }
          return false;
        });
    }
  };
}