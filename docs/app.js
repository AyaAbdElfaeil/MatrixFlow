/**
 * MatrixFlow — Client-side Controller & Dynamic Orchestrator
 * Soft Pastel Aesthetic Edition
 * 
 * Supports both:
 * 1. Native High-Performance C++ Backend (via local /api/solve)
 * 2. Automatic Pure Client-Side Fallback (for static GitHub Pages deployment)
 */

// Application State
const state = {
    currentTab: 'tab-input',
    n: 3,
    m: 3,
    mode: 'REF',
    displayFractions: true,
    equations: [
        { coefficients: ['2', '1', '-1'], rhs: '8' },
        { coefficients: ['-3', '-1', '2'], rhs: '-11' },
        { coefficients: ['-2', '1', '2'], rhs: '-3' }
    ],

    // Solver data
    solverData: null,
    currentStepIndex: 0,
    isPlaying: false,
    playTimer: null,
    playSpeed: 1400
};

// DOM References
const els = {
    // Tabs
    tabBtns: {
        'tab-input': document.getElementById('tab-btn-input'),
        'tab-solve': document.getElementById('tab-btn-solve'),
        'tab-results': document.getElementById('tab-btn-results')
    },
    tabPanes: {
        'tab-input': document.getElementById('tab-input'),
        'tab-solve': document.getElementById('tab-solve'),
        'tab-results': document.getElementById('tab-results')
    },

    // Dimensions Inputs
    inputRows: document.getElementById('input-rows'),
    inputVars: document.getElementById('input-vars'),
    btnIncRows: document.getElementById('btn-inc-rows'),
    btnDecRows: document.getElementById('btn-dec-rows'),
    btnIncVars: document.getElementById('btn-inc-vars'),
    btnDecVars: document.getElementById('btn-dec-vars'),

    // Input Actions
    btnGenerateMatrix: document.getElementById('btn-generate-matrix'),
    btnLoadExample: document.getElementById('btn-load-example'),
    btnReset: document.getElementById('btn-reset'),

    // Equation Table
    tableHead: document.getElementById('table-head'),
    tableBody: document.getElementById('table-body'),
    validationNotice: document.getElementById('validation-notice'),
    validationMsg: document.getElementById('validation-msg'),

    // Mode Selection Pills
    pillRef: document.getElementById('pill-ref'),
    pillRref: document.getElementById('pill-rref'),
    solvePillRef: document.getElementById('solve-pill-ref'),
    solvePillRref: document.getElementById('solve-pill-rref'),

    // Solve Tab
    lblStepBadge: document.getElementById('lbl-step-badge'),
    lblOpNotation: document.getElementById('lbl-op-notation'),
    solveMatrixDisplay: document.getElementById('solve-matrix-display'),
    lblStepTitle: document.getElementById('lbl-step-title'),
    lblStepReason: document.getElementById('lbl-step-reason'),
    lblStepDetail: document.getElementById('lbl-step-detail'),
    btnToggleFormat: document.getElementById('btn-toggle-format'),
    lblFormatName: document.getElementById('lbl-format-name'),
    btnStepPrev: document.getElementById('btn-step-prev'),
    btnStepPlay: document.getElementById('btn-step-play'),
    playBtnText: document.getElementById('play-btn-text'),
    btnStepNext: document.getElementById('btn-step-next'),
    btnStepRestart: document.getElementById('btn-step-restart'),
    btnStepFinal: document.getElementById('btn-step-final'),

    // Results Tab
    resCardSummary: document.getElementById('res-card-summary'),
    resIconStatus: document.getElementById('res-icon-status'),
    resClassTitle: document.getElementById('res-class-title'),
    resSolutionText: document.getElementById('res-solution-text'),
    resClassExplanation: document.getElementById('res-class-explanation'),
    resParametricBox: document.getElementById('res-parametric-box'),
    resParametricLines: document.getElementById('res-parametric-lines'),
    resFinalMatrix: document.getElementById('res-final-matrix'),
    statRankA: document.getElementById('stat-rank-a'),
    statRankAb: document.getElementById('stat-rank-ab'),
    statVarsM: document.getElementById('stat-vars-m'),
    statFreeVars: document.getElementById('stat-free-vars'),
    btnResBackInput: document.getElementById('btn-res-back-input'),
    btnResBackSolve: document.getElementById('btn-res-back-solve'),
    btnCopySolution: document.getElementById('btn-copy-solution'),

    // Toast
    toast: document.getElementById('toast-pill')
};

// ==========================================================================
// Tab Switching
// ==========================================================================
function switchTab(tabId) {
    state.currentTab = tabId;

    if (tabId !== 'tab-solve' && state.isPlaying) {
        togglePlay(false);
    }

    Object.keys(els.tabPanes).forEach(id => {
        if (id === tabId) {
            els.tabPanes[id].classList.add('active');
            els.tabBtns[id].classList.add('active');
        } else {
            els.tabPanes[id].classList.remove('active');
            els.tabBtns[id].classList.remove('active');
        }
    });

    if (tabId === 'tab-solve' && state.solverData) {
        renderCurrentStep();
    } else if (tabId === 'tab-results' && state.solverData) {
        renderResults();
    }
}

// Show subtle toast
function showToast(msg) {
    els.toast.textContent = msg;
    els.toast.classList.remove('hidden');
    setTimeout(() => {
        els.toast.classList.add('hidden');
    }, 2200);
}

// ==========================================================================
// Dimension Management & Equation Storage
// ==========================================================================
function updateDimensions(newN, newM) {
    newN = Math.max(1, Math.min(10, newN));
    newM = Math.max(1, Math.min(10, newM));

    const oldN = state.equations.length;

    // Adjust rows
    if (newN < oldN) {
        state.equations = state.equations.slice(0, newN);
    } else if (newN > oldN) {
        for (let i = oldN; i < newN; i++) {
            const rowCoeffs = [];
            for (let j = 0; j < newM; j++) {
                rowCoeffs.push('0');
            }
            state.equations.push({ coefficients: rowCoeffs, rhs: '0' });
        }
    }

    // Adjust columns
    state.equations.forEach(eq => {
        const curLen = eq.coefficients.length;
        if (newM < curLen) {
            eq.coefficients = eq.coefficients.slice(0, newM);
        } else if (newM > curLen) {
            for (let j = curLen; j < newM; j++) {
                eq.coefficients.push('0');
            }
        }
    });

    state.n = newN;
    state.m = newM;

    els.inputRows.value = newN;
    els.inputVars.value = newM;

    renderEquationTable();
}

// ==========================================================================
// Equation Editor Table Rendering
// ==========================================================================
function getSubscript(num) {
    const subs = ['₀','₁','₂','₃','₄','₅','₆','₇','₈','₉'];
    return String(num).split('').map(d => subs[parseInt(d)] || d).join('');
}

function renderEquationTable() {
    // 1. Build Header
    let theadHtml = '<tr><th class="col-num"></th>';
    for (let c = 0; c < state.m; c++) {
        theadHtml += `<th>x${getSubscript(c + 1)}</th>`;
    }
    theadHtml += '<th class="col-const">Constant</th></tr>';
    els.tableHead.innerHTML = theadHtml;

    // 2. Build Body Rows
    els.tableBody.innerHTML = '';
    state.equations.forEach((eq, r) => {
        const tr = document.createElement('tr');

        // Row Index Label
        const numTd = document.createElement('td');
        numTd.className = 'eq-row-num';
        numTd.textContent = r + 1;
        tr.appendChild(numTd);

        // Coefficient inputs
        eq.coefficients.forEach((val, c) => {
            const td = document.createElement('td');
            const input = document.createElement('input');
            input.type = 'text';
            input.className = 'cell-input';
            input.value = val;
            input.dataset.row = r;
            input.dataset.col = c;

            input.addEventListener('input', (e) => {
                state.equations[r].coefficients[c] = e.target.value.trim();
                input.classList.remove('error');
                hideValidationNotice();
            });

            td.appendChild(input);
            tr.appendChild(td);
        });

        // Constant input
        const constTd = document.createElement('td');
        const constInput = document.createElement('input');
        constInput.type = 'text';
        constInput.className = 'cell-input const-cell-input';
        constInput.value = eq.rhs;
        constInput.dataset.row = r;
        constInput.dataset.col = state.m;

        constInput.addEventListener('input', (e) => {
            state.equations[r].rhs = e.target.value.trim();
            constInput.classList.remove('error');
            hideValidationNotice();
        });

        constTd.appendChild(constInput);
        tr.appendChild(constTd);

        els.tableBody.appendChild(tr);
    });
}

function showValidationNotice(message, cellErrors = []) {
    els.validationMsg.textContent = message;
    els.validationNotice.classList.remove('hidden');

    document.querySelectorAll('.cell-input').forEach(inp => inp.classList.remove('error'));

    cellErrors.forEach(err => {
        const input = document.querySelector(`.cell-input[data-row="${err.row}"][data-col="${err.col}"]`);
        if (input) {
            input.classList.add('error');
            input.title = err.message;
        }
    });
}

function hideValidationNotice() {
    els.validationNotice.classList.add('hidden');
}

// ==========================================================================
// Mode Toggle (REF vs RREF)
// ==========================================================================
function setMode(newMode) {
    state.mode = newMode;

    if (newMode === 'REF') {
        els.pillRef.classList.add('active');
        els.pillRref.classList.remove('active');
        els.solvePillRef.classList.add('active');
        els.solvePillRref.classList.remove('active');
    } else {
        els.pillRref.classList.add('active');
        els.pillRef.classList.remove('active');
        els.solvePillRref.classList.add('active');
        els.solvePillRef.classList.remove('active');
    }
}

// ==========================================================================
// Solver Execution: C++ Backend with Seamless Client-Side Fallback
// ==========================================================================
async function solveMatrix(autoSwitchTab = true) {
    const payload = {
        rows: state.n,
        vars: state.m,
        mode: state.mode,
        equations: state.equations
    };

    let data = null;

    try {
        els.btnGenerateMatrix.disabled = true;

        // Try C++ backend first
        const resp = await fetch('/api/solve', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });

        if (resp.ok) {
            data = await resp.json();
        } else {
            throw new Error('Backend responded with non-200');
        }
    } catch (err) {
        // Fallback to pure client-side Gaussian solver (for static GitHub Pages hosting)
        data = runClientGaussianSolver(payload);
    } finally {
        els.btnGenerateMatrix.disabled = false;
    }

    if (!data.success) {
        if (data.validation) {
            showValidationNotice(data.validation.generalError, data.validation.cellErrors);
        } else {
            showValidationNotice(data.error || 'Failed to solve matrix.');
        }
        return false;
    }

    state.solverData = data;
    state.currentStepIndex = 0;
    hideValidationNotice();

    if (autoSwitchTab) {
        switchTab('tab-solve');
        showToast('Matrix generated and solved!');
    }

    return true;
}

// ==========================================================================
// Step-by-Step Visualization
// ==========================================================================
function renderCurrentStep() {
    if (!state.solverData || !state.solverData.steps) return;

    const steps = state.solverData.steps;
    const step = steps[state.currentStepIndex];
    const totalSteps = steps.length;

    // Update Step Meta
    els.lblStepBadge.textContent = `Step ${state.currentStepIndex} of ${totalSteps - 1}`;
    els.lblOpNotation.textContent = step.notation.replace(/\\leftrightarrow/g, '↔').replace(/\\to/g, '→').replace(/[{}]/g, '');

    // Update Explanations
    els.lblStepTitle.textContent = step.title;
    els.lblStepReason.textContent = step.reason;
    els.lblStepDetail.textContent = step.explanation;

    // Render Matrix with brackets and highlights
    renderMatrixDisplay(els.solveMatrixDisplay, step.matrixAfter, step);

    // Update Buttons
    els.btnStepPrev.disabled = (state.currentStepIndex === 0);
    els.btnStepNext.disabled = (state.currentStepIndex === totalSteps - 1);
    els.btnStepRestart.disabled = (state.currentStepIndex === 0);
}

function renderMatrixDisplay(container, matrixObj, stepMeta = null) {
    container.innerHTML = '';
    const grid = document.createElement('div');
    grid.className = 'matrix-grid';

    const rows = matrixObj.rows;
    const cols = matrixObj.cols;

    for (let r = 0; r < rows; r++) {
        const rowDiv = document.createElement('div');
        rowDiv.className = 'matrix-row';

        if (stepMeta && stepMeta.activeRow === r) {
            rowDiv.classList.add('active-row');
        }

        for (let c = 0; c < cols; c++) {
            if (c === cols - 1 && cols > 1) {
                const sep = document.createElement('div');
                sep.className = 'matrix-separator';
                rowDiv.appendChild(sep);
            }

            const cellDiv = document.createElement('div');
            cellDiv.className = 'matrix-cell';
            if (c === cols - 1) cellDiv.classList.add('rhs-col');

            const cellData = matrixObj.data[r][c];
            cellDiv.textContent = state.displayFractions ? cellData.str : cellData.dec;

            if (stepMeta) {
                if (stepMeta.pivotRow === r && stepMeta.pivotCol === c) {
                    cellDiv.classList.add('pivot-cell');
                    cellDiv.title = 'Leading Pivot';
                } else if (stepMeta.eliminatedRow === r && stepMeta.eliminatedCol === c) {
                    cellDiv.classList.add('eliminated-cell');
                    cellDiv.title = 'Eliminated to Zero';
                }
            }

            rowDiv.appendChild(cellDiv);
        }

        grid.appendChild(rowDiv);
    }

    container.appendChild(grid);
}

// Playback Controls
function togglePlay(forceState = null) {
    state.isPlaying = (forceState !== null) ? forceState : !state.isPlaying;

    if (state.isPlaying) {
        els.btnStepPlay.classList.add('playing');
        els.playBtnText.innerHTML = '&#10074;&#10074; Pause';

        if (state.currentStepIndex >= state.solverData.steps.length - 1) {
            state.currentStepIndex = 0;
            renderCurrentStep();
        }

        startTimer();
    } else {
        els.btnStepPlay.classList.remove('playing');
        els.playBtnText.innerHTML = '&#9654; Play';
        clearTimeout(state.playTimer);
    }
}

function startTimer() {
    clearTimeout(state.playTimer);
    state.playTimer = setTimeout(() => {
        if (!state.isPlaying) return;

        if (state.currentStepIndex < state.solverData.steps.length - 1) {
            state.currentStepIndex++;
            renderCurrentStep();
            startTimer();
        } else {
            togglePlay(false);
            showToast('Elimination demonstration completed.');
        }
    }, state.playSpeed);
}

// ==========================================================================
// Results Tab Rendering
// ==========================================================================
function renderResults() {
    if (!state.solverData) return;

    const classification = state.solverData.classification;

    els.resCardSummary.className = 'results-summary-card';
    if (classification.type === 'UNIQUE') {
        els.resClassTitle.textContent = 'Unique Solution';
        els.resIconStatus.innerHTML = '&#10003;';

        const vals = classification.uniqueSolution.map((s, idx) => {
            const valStr = state.displayFractions ? s.val : s.dec;
            return `x${getSubscript(idx + 1)} = ${valStr}`;
        }).join(',  ');

        els.resSolutionText.textContent = vals;
        els.resClassExplanation.textContent = classification.explanation;
        els.resParametricBox.classList.add('hidden');

    } else if (classification.type === 'INFINITE') {
        els.resCardSummary.classList.add('infinite');
        els.resClassTitle.textContent = `Infinitely Many Solutions (${classification.numFreeVars} Free Variable${classification.numFreeVars > 1 ? 's' : ''})`;
        els.resIconStatus.innerHTML = '&infin;';
        els.resSolutionText.textContent = classification.solutionSummary;
        els.resClassExplanation.textContent = classification.explanation;

        els.resParametricBox.classList.remove('hidden');
        els.resParametricLines.innerHTML = '';
        classification.parametricSolution.forEach(line => {
            const div = document.createElement('div');
            div.textContent = line;
            els.resParametricLines.appendChild(div);
        });

    } else {
        els.resCardSummary.classList.add('inconsistent');
        els.resClassTitle.textContent = 'Inconsistent System (No Solution)';
        els.resIconStatus.innerHTML = '&times;';
        els.resSolutionText.textContent = '0 = c (where c ≠ 0)';
        els.resClassExplanation.textContent = classification.explanation;
        els.resParametricBox.classList.add('hidden');
    }

    renderMatrixDisplay(els.resFinalMatrix, state.solverData.finalMatrix);

    els.statRankA.textContent = classification.rankA;
    els.statRankAb.textContent = classification.rankAb;
    els.statVarsM.textContent = classification.numVars;
    els.statFreeVars.textContent = classification.numFreeVars;
}

function copySolution() {
    if (!state.solverData) return;
    const classification = state.solverData.classification;
    let text = `MatrixFlow Solution:\nSystem Type: ${classification.typeString}\nSummary: ${classification.solutionSummary}\nRank(A): ${classification.rankA}, Rank([A|b]): ${classification.rankAb}\n\n`;

    if (classification.type === 'UNIQUE') {
        text += 'Solution Vector:\n';
        classification.uniqueSolution.forEach((s, idx) => {
            text += `  x${idx + 1} = ${s.val}\n`;
        });
    } else if (classification.type === 'INFINITE') {
        text += 'Parametric Representation:\n';
        classification.parametricSolution.forEach(line => {
            text += `  ${line}\n`;
        });
    }

    navigator.clipboard.writeText(text).then(() => {
        showToast('Solution copied to clipboard!');
    }).catch(() => {
        showToast('Unable to copy to clipboard.');
    });
}

function loadExample() {
    state.n = 3;
    state.m = 3;
    state.mode = 'REF';
    state.equations = [
        { coefficients: ['2', '1', '-1'], rhs: '8' },
        { coefficients: ['-3', '-1', '2'], rhs: '-11' },
        { coefficients: ['-2', '1', '2'], rhs: '-3' }
    ];

    els.inputRows.value = 3;
    els.inputVars.value = 3;
    setMode('REF');
    renderEquationTable();
    hideValidationNotice();
    showToast('Example system loaded.');
}

function resetInputs() {
    state.equations.forEach(eq => {
        eq.coefficients = eq.coefficients.map(() => '0');
        eq.rhs = '0';
    });
    renderEquationTable();
    hideValidationNotice();
    showToast('All inputs cleared.');
}

// ==========================================================================
// Pure JavaScript Exact Rational Arithmetic & Gaussian Fallback Engine
// ==========================================================================
class JSFraction {
    constructor(n = 0n, d = 1n) {
        if (typeof n === 'number') n = BigInt(Math.round(n));
        if (typeof d === 'number') d = BigInt(Math.round(d));
        if (d === 0n) throw new Error('Division by zero');
        if (d < 0n) { n = -n; d = -d; }
        const g = JSFraction.gcd(n < 0n ? -n : n, d);
        this.num = n / g;
        this.den = d / g;
    }

    static gcd(a, b) {
        while (b !== 0n) {
            let t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    static parse(str) {
        str = String(str).trim();
        if (str.includes('/')) {
            const [nStr, dStr] = str.split('/');
            const n = BigInt(nStr.trim());
            const d = BigInt(dStr.trim());
            return new JSFraction(n, d);
        }
        if (str.includes('.')) {
            const val = parseFloat(str);
            let sign = val < 0 ? -1n : 1n;
            let x = Math.abs(val);
            let h1 = 1n, h0 = 0n, k1 = 0n, k0 = 1n, b = x;
            for (let i = 0; i < 20; i++) {
                let a = BigInt(Math.floor(b));
                let h2 = a * h1 + h0;
                let k2 = a * k1 + k0;
                if (k2 > 10000000n) break;
                h0 = h1; h1 = h2; k0 = k1; k1 = k2;
                if (Math.abs(x - Number(h1) / Number(k1)) < 1e-9) break;
                let diff = b - Math.floor(b);
                if (Math.abs(diff) < 1e-12) break;
                b = 1.0 / diff;
            }
            return new JSFraction(sign * h1, k1);
        }
        return new JSFraction(BigInt(str), 1n);
    }

    add(o) { return new JSFraction(this.num * o.den + o.num * this.den, this.den * o.den); }
    sub(o) { return new JSFraction(this.num * o.den - o.num * this.den, this.den * o.den); }
    mul(o) { return new JSFraction(this.num * o.num, this.den * o.den); }
    div(o) { return new JSFraction(this.num * o.den, this.den * o.num); }
    neg() { return new JSFraction(-this.num, this.den); }
    abs() { return new JSFraction(this.num < 0n ? -this.num : this.num, this.den); }
    isZero() { return this.num === 0n; }
    isOne() { return this.num === 1n && this.den === 1n; }
    toDouble() { return Number(this.num) / Number(this.den); }
    toString() { return this.den === 1n ? this.num.toString() : `${this.num}/${this.den}`; }
    toDecimalString() {
        let v = this.toDouble();
        if (Math.abs(v) < 1e-9) v = 0;
        let s = v.toFixed(3);
        if (s.includes('.')) {
            s = s.replace(/0+$/, '').replace(/\.$/, '');
        }
        return s === '-0' ? '0' : s;
    }
    toObj() {
        return {
            num: Number(this.num),
            den: Number(this.den),
            str: this.toString(),
            dec: this.toDecimalString(),
            val: this.toDouble()
        };
    }
}

function runClientGaussianSolver(req) {
    const rows = req.rows;
    const vars = req.vars;
    const cols = vars + 1;
    const mode = req.mode;

    const cellErrors = [];
    const matData = [];

    for (let r = 0; r < rows; r++) {
        const row = [];
        const eq = req.equations[r];
        for (let c = 0; c < vars; c++) {
            try {
                row.push(JSFraction.parse(eq.coefficients[c]));
            } catch (e) {
                cellErrors.push({ row: r, col: c, message: `Equation ${r + 1}, variable x${c + 1}: Invalid number` });
            }
        }
        try {
            row.push(JSFraction.parse(eq.rhs));
        } catch (e) {
            cellErrors.push({ row: r, col: vars, message: `Equation ${r + 1}, RHS constant: Invalid number` });
        }
        matData.push(row);
    }

    if (cellErrors.length > 0) {
        return {
            success: false,
            validation: {
                isValid: false,
                generalError: 'Please correct the highlighted inputs before generating the matrix.',
                cellErrors
            }
        };
    }

    function snapshot(data) {
        return {
            rows,
            cols,
            data: data.map(r => r.map(c => c.toObj()))
        };
    }

    const curMat = matData.map(r => r.map(c => new JSFraction(c.num, c.den)));
    const steps = [];
    let stepCount = 0;

    steps.push({
        stepNumber: stepCount++,
        title: 'Initial Augmented Matrix',
        operationType: 'INITIAL',
        notation: 'None',
        reason: 'Starting augmented matrix [A | b]',
        explanation: `System initialized with ${rows} equations and ${vars} variables.`,
        matrixBefore: snapshot(curMat),
        matrixAfter: snapshot(curMat),
        pivotRow: -1, pivotCol: -1, activeRow: -1, sourceRow: -1, eliminatedRow: -1, eliminatedCol: -1
    });

    const pivots = [];
    let pRow = 0;

    // Forward pass (REF)
    for (let col = 0; col < vars && pRow < rows; col++) {
        let bestRow = pRow;
        let maxVal = curMat[pRow][col].abs().toDouble();

        for (let r = pRow + 1; r < rows; r++) {
            let val = curMat[r][col].abs().toDouble();
            if (val > maxVal) {
                maxVal = val;
                bestRow = r;
            }
        }

        if (maxVal === 0) continue;

        if (bestRow !== pRow) {
            const before = snapshot(curMat);
            const temp = curMat[pRow];
            curMat[pRow] = curMat[bestRow];
            curMat[bestRow] = temp;

            steps.push({
                stepNumber: stepCount++,
                title: `Row Swap: R${pRow + 1} ↔ R${bestRow + 1}`,
                operationType: 'SWAP',
                notation: `R_{${pRow + 1}} \\leftrightarrow R_{${bestRow + 1}}`,
                reason: `Swap Row ${pRow + 1} and Row ${bestRow + 1} to position non-zero pivot.`,
                explanation: `Swapped rows to establish leading coefficient at column ${col + 1}.`,
                matrixBefore: before,
                matrixAfter: snapshot(curMat),
                pivotRow: pRow, pivotCol: col, activeRow: pRow, sourceRow: bestRow, eliminatedRow: -1, eliminatedCol: -1
            });
        }

        pivots.push([pRow, col]);
        const pivotVal = curMat[pRow][col];

        for (let r = pRow + 1; r < rows; r++) {
            const targetVal = curMat[r][col];
            if (!targetVal.isZero()) {
                const factor = targetVal.div(pivotVal);
                const before = snapshot(curMat);

                for (let c = 0; c < cols; c++) {
                    curMat[r][c] = curMat[r][c].sub(factor.mul(curMat[pRow][c]));
                }

                steps.push({
                    stepNumber: stepCount++,
                    title: `Eliminate R${r + 1} Entry`,
                    operationType: 'ELIMINATE',
                    notation: `R_{${r + 1}} \\to R_{${r + 1}} - (${factor.toString()}) R_{${pRow + 1}}`,
                    reason: `Eliminate non-zero entry below pivot at Row ${r + 1}, Col ${col + 1}.`,
                    explanation: `Subtracted (${factor.toString()}) × Row ${pRow + 1} from Row ${r + 1}.`,
                    matrixBefore: before,
                    matrixAfter: snapshot(curMat),
                    pivotRow: pRow, pivotCol: col, activeRow: r, sourceRow: pRow, eliminatedRow: r, eliminatedCol: col
                });
            }
        }

        pRow++;
    }

    // Backward pass (RREF)
    if (mode === 'RREF') {
        for (let p = pivots.length - 1; p >= 0; p--) {
            const [pr, pc] = pivots[p];
            const pVal = curMat[pr][pc];

            if (!pVal.isOne()) {
                const scale = new JSFraction(1n, 1n).div(pVal);
                const before = snapshot(curMat);
                for (let c = 0; c < cols; c++) curMat[pr][c] = curMat[pr][c].mul(scale);

                steps.push({
                    stepNumber: stepCount++,
                    title: `Normalize Pivot at R${pr + 1}`,
                    operationType: 'SCALE',
                    notation: `R_{${pr + 1}} \\to (${scale.toString()}) R_{${pr + 1}}`,
                    reason: `Normalize leading pivot at Row ${pr + 1}, Col ${pc + 1} to 1.`,
                    explanation: `Scaled Row ${pr + 1} by (${scale.toString()}).`,
                    matrixBefore: before,
                    matrixAfter: snapshot(curMat),
                    pivotRow: pr, pivotCol: pc, activeRow: pr, sourceRow: -1, eliminatedRow: -1, eliminatedCol: -1
                });
            }

            for (let r = pr - 1; r >= 0; r--) {
                const target = curMat[r][pc];
                if (!target.isZero()) {
                    const factor = target;
                    const before = snapshot(curMat);
                    for (let c = 0; c < cols; c++) curMat[r][c] = curMat[r][c].sub(factor.mul(curMat[pr][c]));

                    steps.push({
                        stepNumber: stepCount++,
                        title: `Eliminate Above Pivot in R${r + 1}`,
                        operationType: 'ELIMINATE',
                        notation: `R_{${r + 1}} \\to R_{${r + 1}} - (${factor.toString()}) R_{${pr + 1}}`,
                        reason: `Ensure pivot is the sole non-zero entry in Column ${pc + 1}.`,
                        explanation: `Subtracted (${factor.toString()}) × Row ${pr + 1} from Row ${r + 1}.`,
                        matrixBefore: before,
                        matrixAfter: snapshot(curMat),
                        pivotRow: pr, pivotCol: pc, activeRow: r, sourceRow: pr, eliminatedRow: r, eliminatedCol: pc
                    });
                }
            }
        }
    }

    // Classification
    let hasContradiction = false;
    let contradictionRow = 0;
    let rankA = 0;
    let rankAb = 0;
    const pivotColMap = {};

    for (let r = 0; r < rows; r++) {
        let allZeroA = true;
        let leadCol = -1;
        for (let c = 0; c < vars; c++) {
            if (!curMat[r][c].isZero()) {
                allZeroA = false;
                if (leadCol === -1) leadCol = c;
            }
        }
        const zeroB = curMat[r][cols - 1].isZero();

        if (!allZeroA) {
            rankA++;
            rankAb++;
            if (leadCol !== -1) pivotColMap[leadCol] = r;
        } else if (!zeroB) {
            rankAb++;
            if (!hasContradiction) {
                hasContradiction = true;
                contradictionRow = r;
            }
        }
    }

    const freeVars = [];
    for (let c = 0; c < vars; c++) {
        if (pivotColMap[c] === undefined) freeVars.push(c);
    }

    let classification = {};
    if (hasContradiction || rankA < rankAb) {
        classification = {
            type: 'INCONSISTENT',
            typeString: 'INCONSISTENT',
            rankA,
            rankAb,
            numVars: vars,
            numEquations: rows,
            numFreeVars: 0,
            solutionSummary: 'No solution exists (Inconsistent System).',
            explanation: `Row ${contradictionRow + 1} produces a contradiction 0 = ${curMat[contradictionRow][cols - 1].toString()}. Because rank(A) = ${rankA} < rank([A|b]) = ${rankAb}, no solution exists.`,
            uniqueSolution: [],
            parametricSolution: []
        };
    } else if (freeVars.length === 0 && rankA === vars) {
        const uniqueVals = [];
        for (let c = 0; c < vars; c++) {
            const r = pivotColMap[c];
            uniqueVals.push({
                var: `x_${c + 1}`,
                val: curMat[r][cols - 1].toString(),
                dec: curMat[r][cols - 1].toDecimalString()
            });
        }
        classification = {
            type: 'UNIQUE',
            typeString: 'UNIQUE',
            rankA,
            rankAb,
            numVars: vars,
            numEquations: rows,
            numFreeVars: 0,
            solutionSummary: `Unique Solution: ${uniqueVals.map(v => `${v.var} = ${v.val}`).join(', ')}`,
            explanation: `The coefficient matrix has full rank (rank(A) = ${rankA} = ${vars}). All variables are pivot variables.`,
            uniqueSolution: uniqueVals,
            parametricSolution: []
        };
    } else {
        const paramEqs = [];
        const paramNames = {};
        freeVars.forEach((fv, idx) => { paramNames[fv] = `t_${idx + 1}`; });

        for (let c = 0; c < vars; c++) {
            if (pivotColMap[c] === undefined) {
                paramEqs.push(`x_${c + 1} = ${paramNames[c]}  (free, ${paramNames[c]} in R)`);
            } else {
                const r = pivotColMap[c];
                const constTerm = curMat[r][cols - 1];
                let eqStr = `x_${c + 1} = `;
                let hasT = false;
                if (!constTerm.isZero() || freeVars.length === 0) {
                    eqStr += constTerm.toString();
                    hasT = true;
                }
                freeVars.forEach(fv => {
                    const coeff = curMat[r][fv];
                    if (!coeff.isZero()) {
                        const term = coeff.neg();
                        if (term.toDouble() > 0) {
                            if (hasT) eqStr += ' + ';
                            eqStr += term.isOne() ? paramNames[fv] : `${term.toString()}${paramNames[fv]}`;
                        } else {
                            const pos = term.neg();
                            eqStr += ' - ';
                            eqStr += pos.isOne() ? paramNames[fv] : `${pos.toString()}${paramNames[fv]}`;
                        }
                        hasT = true;
                    }
                });
                paramEqs.push(hasT ? eqStr : `x_${c + 1} = 0`);
            }
        }

        classification = {
            type: 'INFINITE',
            typeString: 'INFINITE',
            rankA,
            rankAb,
            numVars: vars,
            numEquations: rows,
            numFreeVars: freeVars.length,
            solutionSummary: `Infinitely many solutions (${freeVars.length} free variables): ${freeVars.map(v => `x_${v + 1} free`).join(', ')}`,
            explanation: `System is consistent with rank(A) = ${rankA} < ${vars} variables. There are ${freeVars.length} free variables.`,
            uniqueSolution: [],
            parametricSolution: paramEqs
        };
    }

    return {
        success: true,
        mode,
        wasAlreadyInForm: false,
        summaryMessage: `Successfully transformed matrix into ${mode} in ${steps.length - 1} operations.`,
        initialMatrix: steps[0].matrixBefore,
        finalMatrix: snapshot(curMat),
        pivots: pivots.map(p => ({ row: p[0], col: p[1] })),
        steps,
        classification
    };
}

// ==========================================================================
// Event Listeners Initialization
// ==========================================================================
function bindEvents() {
    Object.keys(els.tabBtns).forEach(id => {
        els.tabBtns[id].addEventListener('click', () => switchTab(id));
    });

    els.btnIncRows.addEventListener('click', () => updateDimensions(state.n + 1, state.m));
    els.btnDecRows.addEventListener('click', () => updateDimensions(state.n - 1, state.m));
    els.btnIncVars.addEventListener('click', () => updateDimensions(state.n, state.m + 1));
    els.btnDecVars.addEventListener('click', () => updateDimensions(state.n, state.m - 1));

    els.inputRows.addEventListener('change', (e) => updateDimensions(parseInt(e.target.value) || 3, state.m));
    els.inputVars.addEventListener('change', (e) => updateDimensions(state.n, parseInt(e.target.value) || 3));

    els.btnGenerateMatrix.addEventListener('click', () => solveMatrix(true));
    els.btnLoadExample.addEventListener('click', loadExample);
    els.btnReset.addEventListener('click', resetInputs);

    els.pillRef.addEventListener('click', () => setMode('REF'));
    els.pillRref.addEventListener('click', () => setMode('RREF'));
    els.solvePillRef.addEventListener('click', async () => {
        setMode('REF');
        await solveMatrix(false);
        renderCurrentStep();
    });
    els.solvePillRref.addEventListener('click', async () => {
        setMode('RREF');
        await solveMatrix(false);
        renderCurrentStep();
    });

    els.btnToggleFormat.addEventListener('click', () => {
        state.displayFractions = !state.displayFractions;
        els.lblFormatName.textContent = state.displayFractions ? 'Fractions' : 'Decimals';
        if (state.currentTab === 'tab-solve') renderCurrentStep();
        if (state.currentTab === 'tab-results') renderResults();
    });

    els.btnStepPrev.addEventListener('click', () => {
        if (state.currentStepIndex > 0) {
            state.currentStepIndex--;
            renderCurrentStep();
        }
    });

    els.btnStepNext.addEventListener('click', () => {
        if (state.solverData && state.currentStepIndex < state.solverData.steps.length - 1) {
            state.currentStepIndex++;
            renderCurrentStep();
        }
    });

    els.btnStepRestart.addEventListener('click', () => {
        state.currentStepIndex = 0;
        renderCurrentStep();
    });

    els.btnStepPlay.addEventListener('click', () => togglePlay());

    els.btnStepFinal.addEventListener('click', () => {
        if (state.solverData) {
            state.currentStepIndex = state.solverData.steps.length - 1;
            renderCurrentStep();
            switchTab('tab-results');
        }
    });

    els.btnResBackInput.addEventListener('click', () => switchTab('tab-input'));
    els.btnResBackSolve.addEventListener('click', () => switchTab('tab-solve'));
    els.btnCopySolution.addEventListener('click', copySolution);

    window.addEventListener('keydown', (e) => {
        if (state.currentTab === 'tab-solve' && !['INPUT', 'SELECT'].includes(document.activeElement.tagName)) {
            if (e.key === 'ArrowRight') {
                e.preventDefault();
                els.btnStepNext.click();
            } else if (e.key === 'ArrowLeft') {
                e.preventDefault();
                els.btnStepPrev.click();
            } else if (e.key === ' ') {
                e.preventDefault();
                togglePlay();
            }
        }
    });
}

// Initial Boot
window.addEventListener('DOMContentLoaded', () => {
    bindEvents();
    renderEquationTable();
    solveMatrix(false);
});
