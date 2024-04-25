const container = document.getElementById('container');
const numberOfCubes = calculateNumberOfCubes();
const rows = Math.ceil(Math.sqrt(numberOfCubes / 1.5));
const cols = Math.ceil(numberOfCubes / rows);
const maxPullDistance = 150; // Maximum distance for cube pull reset
const maxFadeDistance = 300; // Maximum distance for cube fade reset

function calculateNumberOfCubes() {
    const screenWidth = window.innerWidth;
    const screenHeight = window.innerHeight;
    const cubeSize = 20;
    const horizontalCubes = Math.ceil(screenWidth / cubeSize);
    const verticalCubes = Math.ceil(screenHeight / cubeSize);
    return horizontalCubes * verticalCubes;
}

// Function to calculate fading color based on distance
function calculateColor(distance) {
    const defaultColor = '#000000'; // External color
    const targetColor = '#16031f'; // Internal for fade effect

    const defaultRgb = hexToRgb(defaultColor);
    const targetRgb = hexToRgb(targetColor);

    const r	= targetRgb.r + (defaultRgb.r - targetRgb.r) * distance / maxFadeDistance;
    const g	= targetRgb.g + (defaultRgb.g - targetRgb.g) * distance / maxFadeDistance;
    const b = targetRgb.b + (defaultRgb.b - targetRgb.b) * distance / maxFadeDistance;

    return `rgb(${r}, ${g}, ${b})`;
}


function hexToRgb(hex) {
    hex = hex.replace('#', '');

    const bigint = parseInt(hex, 16);
    const r = (bigint >> 16) & 255;
    const g = (bigint >> 8) & 255;
    const b = bigint & 255;

    return { r, g, b };
}


// Calculate initial position of each cube
for (let i = 0; i < numberOfCubes; i++) {
    const cube = document.createElement('div');
    cube.classList.add('cube');
    const row = Math.floor(i / cols);
    const col = i % cols;
    const x = col * 25;
    const y = row * 25;
    cube.dataset.initialX = x;
    cube.dataset.initialY = y;
    cube.style.transform = `translate3d(${x}px, ${y}px, 0)`;
    container.appendChild(cube);
}

const cubes = document.querySelectorAll('.cube');
const defaultColor = '#000000'; // Default cube color

document.addEventListener('mousemove', (e) => {
    const { clientX, clientY } = e;
    cubes.forEach(cube => {
        const cubeX = parseFloat(cube.dataset.initialX);
        const cubeY = parseFloat(cube.dataset.initialY);
        const deltaX = (cubeX - clientX) / 10;
        const deltaY = (cubeY - clientY) / 10;
        const distance = Math.sqrt((cubeX - clientX) ** 2 + (cubeY - clientY) ** 2);
        if (distance < maxPullDistance) {
            const scaleFactor = 1 - distance / maxPullDistance;
            const newX = cubeX - deltaX * scaleFactor;
            const newY = cubeY - deltaY * scaleFactor;
            cube.style.transform = `translate(${newX}px, ${newY}px) rotateX(${deltaY}deg) rotateY(${deltaX}deg)`;
        } else {
            cube.style.transform = `translate(${cubeX}px, ${cubeY}px)`;
        }
        if (distance < maxFadeDistance) {
            cube.style.backgroundColor = calculateColor(distance);
        } else {
        		cube.style.backgroundColor = defaultColor;
        }
    });
});