class React {
	constructor() {
		console.log("Constructed");
	}

	
	useState(initial) {
		let state = {
			value: initial
		};
		
		const setState = (newState) => {
			state.value = newState;
			this.render();
		}
		return [state, setState];
	}
	
	render() {
		
	}
}

export default React;