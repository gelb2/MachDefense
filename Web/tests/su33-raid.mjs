import assert from 'node:assert/strict';
import fs from 'node:fs/promises';
import path from 'node:path';
import {chromium, webkit} from 'playwright';

const [output, url, mode = 'cheat'] = process.argv.slice(2);
const engine = process.env.MD_BROWSER || 'chromium';
await fs.mkdir(output, {recursive: true});
const browser = await (engine === 'webkit' ? webkit : chromium).launch({
    headless: true,
    ...(engine === 'chromium' ? {executablePath: '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'} : {}),
});
const page = await browser.newPage({viewport: {width: 420, height: 880}});
const errors = [];
const result = {engine, mode, scenarios: [], errors};
page.on('pageerror', error => errors.push(error.message));

const scenarios = [
    'center', 'top', 'bottom', 'left', 'right',
    'top-left', 'top-right', 'bottom-left', 'bottom-right',
    'outside', 'moving', 'light-helicopter-evasion',
];

async function rawCommand(command, argument) {
    return page.evaluate(([code, value]) =>
        MDTest.call('md_aircraft_scene', ['number', 'number'], [code, value], 'string'), [command, argument]);
}
async function sceneCommand(command, argument) {
    const response = await rawCommand(command, argument);
    assert.ok(response.startsWith('{'), response);
    return JSON.parse(response);
}

async function checkScenario(index, name) {
    const first = await sceneCommand(30, index);
    let state = first;
    let observedPlane = false;
    let observedOnScreenPlane = false;
    let observedWarning = false;
    let observedBeam = false;
    let maxTravel = 0;
    let capture = false;
    for (let step = 0; step < 500 && !state.exited; ++step) {
        observedPlane ||= state.planeVisuals === 1;
        observedOnScreenPlane ||= state.planeOnScreen;
        observedWarning ||= state.warningVisuals > 0;
        observedBeam ||= state.beamVisuals > 0;
        maxTravel = Math.max(maxTravel, Math.hypot(
            state.targetPosition[0] - first.initialPosition[0],
            state.targetPosition[1] - first.initialPosition[1],
        ));
        if (!capture && state.planeOnScreen && state.laserShots >= 12 && state.beamVisuals > 0 &&
            (index === 0 || index === 10)) {
            await page.locator('#game').screenshot({path: path.join(output, `${name}.png`)});
            capture = true;
        }
        state = await sceneCommand(31, index === 0 ? 2 : 4);
    }
    const report = {name, ...state, observedPlane, observedOnScreenPlane, observedWarning, observedBeam, maxTravel, capture};
    result.scenarios.push(report);
    if (index < 10 || index >= 100) {
        assert.deepEqual(state.targetPosition, first.initialPosition, name);
        assert.equal(state.appliedLaserDamage, 189, name);
        assert.equal(first.maxHP - state.hp, 378, name);
    } else if (index === 10 || index === 11) {
        assert.ok(maxTravel > 1, name);
        assert.ok(state.appliedLaserDamage <= 189, name);
    }
    if (index === 11) assert.ok(state.dodges > 0, name);
    assert.equal(state.writes, 0, name);
    assert.equal(state.laserShots, 14, name);
    assert.equal(state.leftShots, 7, name);
    assert.equal(state.rightShots, 7, name);
    assert.equal(state.wingSequence, 'LRLRLRLRLRLRLR', name);
    assert.equal(state.missiles, 2, name);
    assert.equal(state.budget, 378, name);
    assert.equal(state.scheduledDamage, state.budget, name);
    assert.ok(first.maxHP - state.hp <= state.budget, name);
    assert.ok(state.lastLaserAt > state.firstLaserAt, name);
    assert.equal(state.laserSounds, 14, name);
    assert.ok(state.closestWhileFiring > 10, name);
    assert.equal(state.earlyClose, false, name);
    assert.equal(state.distanceAdjustments, 0, name);
    assert.ok(observedPlane && observedWarning && observedBeam, name);
    assert.ok(state.exited && !state.active && state.planeVisuals === 0, name);
    const duration = state.lastLaserAt - state.firstLaserAt;
    if (index === 0) {
        assert.ok(Math.abs(duration - 1.8) <= 0.0167, name);
    }
    if (index === 10) {
        const staticCenter = result.scenarios[0];
        assert.ok(duration < staticCenter.lastLaserAt - staticCenter.firstLaserAt, name);
    }
}

try {
    await page.goto(url);
    await page.waitForSelector('#play:not([disabled])', {timeout: 120000});
    await page.click('#play');
    await page.waitForFunction(() => window.MDModule && document.querySelector('#launcher').hidden);
    await page.evaluate(() => MDTest.advance(1));
    const start = await page.evaluate(() => MDTest.state());
    assert.equal(start.cheat, mode === 'cheat');
    if (mode === 'normal') {
        for (const command of [30, 31, 32]) {
            assert.equal(await rawCommand(command, 0), 'Unavailable');
        }
    } else {
        for (const [index, name] of scenarios.entries()) await checkScenario(index, name);
        await checkScenario(100, 'pad-center');
        await checkScenario(200, 'wide-phone-center');
        assert.ok(result.scenarios[0].capture, 'Center beam screenshot');
        assert.ok(result.scenarios[10].capture, 'Moving target beam screenshot');
    }
    const end = await page.evaluate(() => MDTest.state());
    assert.equal(end.writes, start.writes);
    assert.deepEqual(errors, []);
    result.ok = true;
} catch (error) {
    result.error = error.stack;
    await page.screenshot({path: path.join(output, 'failure.png')});
    process.exitCode = 1;
} finally {
    await fs.writeFile(path.join(output, 'su33-raid.json'), JSON.stringify(result, null, 2));
    console.log(JSON.stringify(result));
    await browser.close();
}
